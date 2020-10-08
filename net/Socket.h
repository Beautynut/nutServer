#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <endian.h>
#include "../base/Logging.h"
#include "Channel.h"

namespace nut
{
class EventLoop;
// 封装了sockaddr_in
class inetAddr
{
    public:
        inetAddr(int port)
        {
            bzero(&inetAddr_, sizeof(inetAddr_));
            inetAddr_.sin_family = AF_INET;
            inetAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
            inetAddr_.sin_port = htons(static_cast<unsigned short>(port));
        }
        inetAddr(const struct sockaddr_in& InetAddr)
            :inetAddr_(InetAddr)
        {}
        ~inetAddr()
        {}
        void setSockAddrInet(const struct sockaddr_in& addr) { inetAddr_ = addr; }
        const struct sockaddr_in& getInetAddr() const { return inetAddr_; }
        std::string getAddrString() const;
        std::string toHostPort() const;
    private:
        struct sockaddr_in inetAddr_;
};

// 封装了socket
class Socket
{
    public:
        Socket();
        explicit Socket(int fd)
            :sockFd_(fd)
        {}
        ~Socket();

        int fd(){ return sockFd_; }
        int bindAddr(const struct inetAddr& serveraddr);
        int listenSocket();
        int accept(inetAddr* peeraddr);
        void setReuseAddr(bool on);
    private:
        int sockFd_;
};

// accpetor监听绑定好的端口,接受新连接,新连接到来时调用newConnectionCb_回调函数
class Acceptor
{
    public:
        typedef boost::function<void(int sockfd,const inetAddr&)> NewConnectionCallback;
        Acceptor(EventLoop* loop,const inetAddr& serveraddr);
        ~Acceptor();

        void listen();
        bool listenning(){ return listenning_; }
        void setNewConnectionCallback(const NewConnectionCallback& cb)
        { newConnectionCb_ = cb; }
    private:
        void handleRead();

        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCb_;
        bool listenning_;
};

}

#endif