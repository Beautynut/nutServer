#ifndef SERVER_HTTPDATA_H
#define SERVER_HTTPDATA_H

#include <string>
#include <map>
namespace nut
{
enum HttpRequestParseState
{
    ExceptRequestLine = 0,
    ExceptRequestHeaders,
    ExceptRequestBody,
    ParseReqFinished
};
enum HttpReqLineParseState
{
    ExceptMethod = 0,
    ExceptUri,
    ExceptVer,
    ParseLineFinished
};
// enum HttpReqHeadersParseState
// {
//     ExceptKey = 0,
//     ExceptValue,
//     ExceptCrlf,
//     ParseHeadersFinished
// };
enum Method
{
    MethodGet = 0,
    MethodHead,
    MethodPut,
    MethodPost,
    MethodTrace,
    MethodOptions,
    MethodDelete
};
enum HttpVersion
{
    Http_09,
    Http_10,
    Http_11
};
class Buffer;
class HttpRequest
{
    public:
        HttpRequest();
        ~HttpRequest();
        bool parseRequest(Buffer* buf);
        bool parseReqLine(const char* start,const char* end);
        bool parseReqUri(const char* start,const char* end);
        bool parseReqHeaders(const char* start,const char* end,Buffer* buf);
        bool ParseReqBody(const char* start,const char* end);

        bool getFullReq()
        { return reqParseState_ == ParseReqFinished; }
        std::string getHeaders(const std::string& key);

        Method getMethod()
        { return method_; }

        HttpVersion getHttpVersion()
        { return version_; }


    private:
        HttpRequestParseState reqParseState_;
        HttpReqLineParseState lineParseState_;

        Method method_;
        std::string uri_;
        HttpVersion version_;
        std::map<std::string ,std::string> requestHeaders_; 
        std::string requestBody_;
};

class HttpResponse
{
    public:
        enum HttpStatusCode
        {
            kUnknown,
            k200_Ok = 200,
            k301_MovedPermanently = 301,
            k303_SeeOther = 303,
            k400_BadRequest = 400,
            k404_NotFound = 404,
            k503_ServiceUnAvailable = 503
        };
        HttpResponse();
        ~HttpResponse();

        void setResponse(HttpRequest* req);
        void addHeader(const std::string& key, const std::string& value)
        { headers_[key] = value; }
        void setBody(const std::string& body)
        { body_ = body; }
        void appendToBuffer(Buffer* output);

        bool closeConnection()
        { return closeConn_; }
    private:
        bool closeConn_;

        std::string line_;
        std::map<std::string,std::string> headers_;
        std::string body_;
};


} // namespace nut


#endif