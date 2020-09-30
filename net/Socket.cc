#include "Socket.h"
#include <boost/bind.hpp>
#include <boost/implicit_cast.hpp>
#include "EventLoop.h"

using namespace nut;

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const SA*>(boost::implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<SA*>(boost::implicit_cast<void*>(addr));
}
Socket::Socket()
{
    sockFd_ =::socket(AF_INET,
                    SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                    IPPROTO_TCP);
    if (sockFd_ < 0)
    {
        LOG<< "sockets::createNonblockingOrDie";
    }
}

Socket::~Socket()
{
    close(sockFd_);
}

int Socket::bindAddr(const struct inetAddr& serveraddr)
{
    struct sockaddr_in addr = serveraddr.getInetAddr();
    int ret = ::bind(sockFd_,sockaddr_cast(&addr),sizeof(addr));
    if(ret < 0)
    {
        LOG << "bind error";
    }
    return ret;
}

int Socket::listenSocket()
{
    int ret = ::listen(sockFd_,2048);
    if(ret < 0)
    {
        LOG << "listen error";
    }
    return ret;
}

int Socket::accept(inetAddr* peeraddr)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof addr;
  bzero(&addr, sizeof addr);
  int connfd = ::accept4(sockFd_, sockaddr_cast(&addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0)
  {
    peeraddr->setSockAddrInet(addr);
  }
  else
  {
      LOG<<"accept error";
  }
  
  return connfd;
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof optval);
}

Acceptor::Acceptor(EventLoop* loop,const inetAddr& serveraddr)
    :loop_(loop),
    acceptSocket_(),
    acceptChannel_(loop_,acceptSocket_.fd()),
    listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddr(serveraddr);
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor()
{
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listenSocket();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    // TODO:防止文件描述符耗尽怎么办

    loop_->assertInLoopThread();
    inetAddr perrAddr(0);
    // 每次accept一个链接,用于长连接
    // TODO:对短链接进行优化
    int connfd = acceptSocket_.accept(&perrAddr);
    if(connfd >= 0)
    {
        if(newConnectionCb_)
        {
            newConnectionCb_(connfd, perrAddr);
        }
    }
    else
    {
        if(::close(connfd) < 0)
        {
            LOG << "close error";
        }
    }
}
