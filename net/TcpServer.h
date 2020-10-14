#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <string>
#include <map>

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Channel.h"
#include "Socket.h"
#include "Buffer.h"
#include "HttpData.h"

namespace nut
{
class EventLoop;
class Aceptor;
class EventLoopThreadPool;
// TcpConnection使用已经创立好连接的sockFd进行初始化
class TcpConnection :   boost::noncopyable,
                        public boost::enable_shared_from_this<TcpConnection>
{
    public:
        typedef boost::function<void()> Callback;
        typedef boost::shared_ptr<TcpConnection> TcpConnSharedPtr;
        typedef boost::function<void(const TcpConnSharedPtr&)> TcpConnCallback;
        typedef boost::function<void (const TcpConnSharedPtr&,
                                Buffer* buf,
                                Timestamp)> MessageCallback;
        typedef boost::function<void (const TcpConnSharedPtr&)> CloseCallback;

        TcpConnection(EventLoop* loop,
                     const std::string& name,
                     int sockfd,
                     const inetAddr& localAddr,
                     const inetAddr& peerAddr);
        ~TcpConnection();

        EventLoop* getLoop() const { return loop_; }
        const std::string& name() const { return name_; }
        const inetAddr& localAddress() { return localAddr_; }
        const inetAddr& peerAddress() { return peerAddr_; }
        bool connected() const { return state_ == Connected; }
        void send(const std::string& messages);
        void shutdown();
        void closeConnection();

        void setConnectionCallback(const TcpConnCallback& cb)
        { tcpConnCallback_ = cb; }

        void setMessageCallback(const MessageCallback& cb)
        { messageCallback_ = cb; }

         void setCloseCallback(const CloseCallback& cb)
        { closeCallback_ = cb; }

        void connectEstablished();

        void connectDestroyed(); 

        HttpRequest* getMutableContext()
        { return &request_; }

        void setReq(const HttpRequest& req)
        { request_ = req; }
    private:
        enum State { Connecting, Connected, Disconnetcing, Disconnected};

        void setState(State stat){ state_ = stat; }
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();
        void sendInLoop(const std::string& message);
        void shutdownInLoop();
        void closeConnectionInLoop();

        State state_;
        EventLoop* loop_;
        std::string name_;
        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;

        inetAddr localAddr_;
        inetAddr peerAddr_;

        TcpConnCallback tcpConnCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;

        HttpRequest request_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;
};

class TcpServer : boost::noncopyable
{
    public:
        typedef boost::function<void()> Callback;
        typedef boost::shared_ptr<TcpConnection> TcpConnSharedPtr;
        typedef boost::function<void(const TcpConnSharedPtr&)> TcpConnCallback;
        typedef boost::function<void (const TcpConnSharedPtr&,
                                Buffer* buf,
                                Timestamp)> MessageCallback;

        TcpServer(EventLoop* loop, const inetAddr& listenAddr);
        ~TcpServer();
        void start();

        EventLoop* getLoop()
        { return loop_; }

        void setConnectionCallback(const TcpConnCallback& cb)
        { connectionCallback_ = cb; }

        void setMessageCallback(const MessageCallback& cb)
        { messageCallback_ = cb; }

        void setThreadNums(int nums);

    private:
        void newConnection(int sockfd, const inetAddr& peerAddr);
        void removeConnection(const TcpConnSharedPtr& conn);
        void removeConnectionInLoop(const TcpConnSharedPtr& conn);

        typedef std::map<std::string, TcpConnSharedPtr> ConnectionMap;

        EventLoop* loop_;
        const std::string name_;
        boost::scoped_ptr<Acceptor> acceptor_;
        boost::scoped_ptr<EventLoopThreadPool> threadPool_;
        TcpConnCallback connectionCallback_;
        MessageCallback messageCallback_;
        bool started_;
        int nextConnId_;
        ConnectionMap connections_;
};

}

#endif