#include "EventLoop.h"
#include "TcpServer.h"
#include "HttpData.h"
#include "HttpServer.h"
#include "../base/Logging.h"

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <iostream>

using namespace nut;

HttpServer::HttpServer(EventLoop* loop,
                    const inetAddr& listenAddr)
            :tcpServer_(loop,listenAddr)
{
    tcpServer_.setConnectionCallback(
        boost::bind(&HttpServer::onConnection, this, _1));
    tcpServer_.setMessageCallback(
        boost::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer()
{

}

void HttpServer::start()
{
    LOG << "HttpServer starts listening";
    tcpServer_.start();
}

void HttpServer::onConnection(const TcpConnSharedPtr& conn)
{
    LOG << "onConnection():new connection from" << conn->peerAddress().toHostPort().c_str();
    if(conn->connected())
    {
        conn->setReq(HttpRequest());
    }
}

void HttpServer::onMessage(const TcpConnSharedPtr& conn,
                Buffer* buf,
                Timestamp recvTime)
{

    HttpRequest* req = conn->getMutableContext();

    if (!req->parseRequest(buf))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }


    if(req->getFullReq())
    {
        onRequest(conn,req);
    }
}

void HttpServer::onRequest(const TcpConnSharedPtr& conn,HttpRequest* req)
{
    HttpResponse response;
    response.setResponse(req);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(buf.retrieveAsString());
    //right :response.closeConnection()
    if(!response.closeConnection())
    {
        conn->shutdown();
    }
}
