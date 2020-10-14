#ifndef SERVER_HTTPSERVER_H
#define SERVER_HTTPSERVER_H

#include "Socket.h"
#include "TcpServer.h"

#include <string>
#include <boost/shared_ptr.hpp>
namespace nut
{
class EventLoop;
class HttpRequest;
class HttpServer
{
    public:
        typedef boost::shared_ptr<TcpConnection> TcpConnSharedPtr;
        // typedef boost::function<void(const HttpRequest& req,
        //                         HttpResponse* response)> HttpCallback;
        HttpServer(EventLoop* loop,
                    const inetAddr& listenAddr);
        ~HttpServer();

        EventLoop* getLoop()
        { return tcpServer_.getLoop(); }

        void setThreadNum(int num)
        { tcpServer_.setThreadNums(num); }

        void start();
    private:
        void onConnection(const TcpConnSharedPtr& conn);
        void onMessage(const TcpConnSharedPtr& conn,
                        Buffer* buf,
                        Timestamp recvTime);
        void onRequest(const TcpConnSharedPtr& conn,HttpRequest* req);

        TcpServer tcpServer_; 
};
}

#endif