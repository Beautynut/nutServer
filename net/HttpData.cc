#include "Buffer.h"
#include "HttpData.h"
#include <string.h>
#include <ctype.h>

using namespace nut;

HttpRequest::HttpRequest()
    :reqParseState_(ExceptRequestLine),
    lineParseState_(ExceptMethod),
    version_(Http_11)
{
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::parseReqLine(const char* start, const char* end)
{
    bool success = false;
    bool finished = false;
    const char* begin = start;
    while (!finished)
    {
        switch (lineParseState_)
        {
            case ExceptMethod:
            {
                const char* space = std::find(start,end,' ');
                if(space == end)
                {
                    finished = true;
                    break;
                }
                begin = space + 1;
                switch (space - begin)
                {
                    case 3:
                        if(strncmp(begin,"GET",3) == 0)
                        {
                            method_ = MethodGet;
                            lineParseState_ = ExceptUri;
                        }
                        else if(strncmp(begin,"PUT",3) == 0)
                        {
                            method_ = MethodPut;
                            lineParseState_ = ExceptUri;   
                        }
                        break;
                    case 4:
                        if(strncmp(begin,"HEAD",4) == 0)
                        {
                            method_ = MethodHead;
                            lineParseState_ = ExceptUri;
                        }
                        else if(strncmp(begin,"POST",4) == 0)
                        {
                            method_ = MethodPost;
                            lineParseState_ = ExceptUri;   
                        }
                        break;
                    case 5:
                        if(strncmp(begin,"TRACE",5) == 0)
                        {
                            method_ = MethodTrace;
                            lineParseState_ = ExceptUri;
                        }
                        break;
                    case 6:
                        if(strncmp(begin,"DELETE",6) == 0)
                        {
                            method_ = MethodDelete;
                            lineParseState_ = ExceptUri;
                        }
                        break;
                    case 7:
                        if(strncmp(begin,"OPTIONS",7) == 0)
                        {
                            method_ = MethodOptions;
                            lineParseState_ = ExceptUri;
                        }
                        break;
                    default:
                        finished = true;
                        break;
                }
                break;
            }
            case ExceptUri:
            {
                const char* UriSpace = std::find(begin,end,' ');
                if(UriSpace != end)
                {
                    // TODO:ParseUri
                    uri_.assign(begin,UriSpace);
                    lineParseState_ = ExceptVer;
                    begin = UriSpace + 1;
                }
                else
                {
                    finished = true;
                }
                break;
            }
            case ExceptVer:
            {
                if(strncmp(begin,"HTTP/1.0",8) == 0)
                {
                    version_ = Http_10;
                    lineParseState_ = ParseLineFinished;
                }
                else if(strncmp(begin,"HTTP/1.1",8) == 0)
                {
                    version_ = Http_11;
                    lineParseState_ = ParseLineFinished;
                }
                else
                {
                    version_ = Http_09;
                    lineParseState_ = ParseLineFinished;
                }
                break;
            }
            case ParseLineFinished:
                finished = true;
                success = true;
                break;
        }
    }
    return success; 
}

bool HttpRequest::parseReqHeaders(const char* start,const char* end,Buffer* buf)
{
    const char* begin = start;
    const char* crlf = buf->findCRLF();
    bool result = false;
    while(crlf != end)
    {
        const char* colon = std::find(begin,crlf,':');
        std::string field(begin, colon);
        ++colon;
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1]))
        {
        value.resize(value.size()-1);
        }
        requestHeaders_[field] = value;
        //TODO: maybe not perfect
        begin = crlf + 2;
        crlf = buf->findCRLF(begin);
        if((crlf - begin) <= 2)
        {
            result = true;
            break;
        }
    }
    return result;
}
bool HttpRequest::ParseReqBody(const char* start ,const char* end)
{
    return true;
}
// TODO: lots of questions
bool HttpRequest::parseRequest(Buffer* buf)
{
    bool finished = false;
    const char* start = buf->peek();
    const char* end = start;
    while (!finished)
    {
        switch (reqParseState_)
        {
            case ExceptRequestLine:
                end = buf->findCRLF();
                if(parseReqLine(start,end))
                {
                    start = end + 2;
                    reqParseState_ = ExceptRequestHeaders;
                }
                else
                {
                    finished = true;
                }
                break;
            case ExceptRequestHeaders:
                end = buf->beginWrite();
                if(parseReqHeaders(start,end,buf))
                {
                    start = end;
                    reqParseState_ = ExceptRequestBody;
                }
                else
                {
                    finished = true;
                }
                break;
            case ExceptRequestBody:
                if(ParseReqBody(start,end))
                {
                    reqParseState_ = ParseReqFinished;
                }
                else
                {
                    finished = true;
                }
                break;
            case ParseReqFinished:
                break;
        }
    }
    return (reqParseState_ == ParseReqFinished); 
}

std::string HttpRequest::getHeaders(const std::string& key)
{
    std::string result;
    std::map<std::string, std::string>::const_iterator it = requestHeaders_.find(key);
    if (it != requestHeaders_.end())
    {
      result = it->second;
    }
    return result;
}

HttpResponse::HttpResponse()
    :closeConn_(true)
{

}

HttpResponse::~HttpResponse()
{

}

// TODO:这一部分还有大量工作尚未完成,目前的响应报文比较简陋
void HttpResponse::setResponse(HttpRequest* req)
{
    switch (req->getHttpVersion())
    {
        case Http_09:
            line_ = "HTTP/0.9";
            line_ += " ";
            break;
        case Http_10:
            line_ = "HTTP/1.0";
            line_ += " ";
            break;
        case Http_11:
            line_ = "HTTP/1.1";
            line_ += " ";
            break;
        default:
            break;
    }

    line_ += "200 OK";

    setBody("Hi, bro, I'm glad to recv your connection,I need your advice :-)");
    
    addHeader("Content-Type","text/plain");

    if(req->getHeaders("Connection") == "close")
    {
        addHeader("Connection","close");
        closeConn_ = true;
    }
    else
    {
        addHeader("Content-Length",std::to_string(body_.size()));
        addHeader("Connection","Keep-Alive");
        closeConn_ = false;
    }
    
}

void HttpResponse::appendToBuffer(Buffer* output)
{
    output->append(line_);
    output->append("\r\n");

    for (const auto& header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }
    output->append("\r\n");
    output->append(body_);
}

