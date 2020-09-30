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
        ~inetAddr()
        {}
        void setSockAddrInet(const struct sockaddr_in& addr) { inetAddr_ = addr; }
        const struct sockaddr_in& getInetAddr() const { return inetAddr_; }
    private:
        struct sockaddr_in inetAddr_;
};

class Socket
{
    public:
        Socket();
        ~Socket();

        int fd(){ return sockFd_; }
        int bindAddr(const struct inetAddr& serveraddr);
        int listenSocket();
        int accept(inetAddr* peeraddr);
        void setReuseAddr(bool on);
    private:
        int sockFd_;
};

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