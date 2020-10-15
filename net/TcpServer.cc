#include "../base/util.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

#include <boost/bind.hpp>

using namespace nut;

TcpConnection::TcpConnection(EventLoop* loop,
                            const std::string& name,
                            int sockfd,
                            const inetAddr& localAddr,
                            const inetAddr& peerAddr)
    :state_(Connecting),
    loop_(loop),
    name_(name),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop,sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{  
    channel_->setReadCallback(
        boost::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
        boost::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        boost::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        boost::bind(&TcpConnection::handleError, this));

}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == Connecting);
  setState(Connected);
  channel_->enableReading();
  tcpConnCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  assert(state_ == Connected);
  setState(Disconnected);
  channel_->disableAll();
  tcpConnCallback_(shared_from_this());

  loop_->removeChannel(get_pointer(channel_));
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
  int savedErrno = 0;
  // 根据read返回的值,调用不同的回调
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
      ssize_t nWrote = ::write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
      if(nWrote > 0)
      {
        outputBuffer_.retrieve(nWrote);
        if(outputBuffer_.readableBytes() == 0)
        {
          channel_->disableWriting();
        }
      }
    }
}

// closeCallback_绑定到TcpServer::removeConnection
void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  LOG << "TcpConnection::handleClose state = " << state_;
  assert(state_ == Connected);
  channel_->disableAll();
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
  int err = getSocketError(channel_->fd());
  LOG << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::send(const std::string& messages)
{
    LOG << "TcpConnection send message:\n"<< messages <<'\n';
    if(state_ == Connected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(messages);
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, messages));
        }    
    }
}

void TcpConnection::sendInLoop(const std::string& messages)
{
    loop_->assertInLoopThread();

    ssize_t nWrote = 0;
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
      nWrote = ::write(channel_->fd(), messages.data(),messages.size());
      if(nWrote < 0)
      {
        nWrote = 0;
        if( errno != EWOULDBLOCK)
        {
          LOG<<"TcpConnection::sendInLoop";
        }
      }
    }

      if(static_cast<size_t>(nWrote) < messages.size())
      {
        outputBuffer_.append(messages.data()+nWrote,messages.size()-nWrote);
        if(!channel_->isWriting())
        {
          channel_->enableWriting();
          if(state_ == Disconnetcing)
          {
            shutdownInLoop();
          }
        }
      }
}

void TcpConnection::shutdown()
{
    if(loop_->isInLoopThread())
    {
        shutdownInLoop();
    }
    else
    {
        loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop,this));
    }
    
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
    {
        if(::shutdown(socket_->fd(),SHUT_WR) < 0)
        {
            LOG<<"TcpCon:shutdownInLoop error";
        }
    }
}

void TcpConnection::closeConnection()
{
    if(state_ == Connected || state_ == Disconnetcing)
    {
        setState(Disconnetcing);
        loop_->queueInLoop(boost::bind(&TcpConnection::closeConnectionInLoop,this));
    }
}

void TcpConnection::closeConnectionInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == Connected || state_ == Disconnetcing)
    {
      handleClose();
    }
}




TcpServer::TcpServer(EventLoop* loop, const inetAddr& listenAddr)
  : loop_((loop)),
    name_(listenAddr.getAddrString()),
    acceptor_(new Acceptor(loop, listenAddr)),
    threadPool_(new EventLoopThreadPool(loop)),
    started_(false),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
    }

    if (!acceptor_->listenning())
    {
        loop_->runInLoop(
            boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::setThreadNums(int nums)
{
    threadPool_->setThreadNum(nums);
}

void TcpServer::newConnection(int sockfd, const inetAddr& peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG << "TcpServer::newConnection [" << name_
            << "] - new connection [" << connName
            << "] from " << peerAddr.getAddrString();
    inetAddr localAddr(getLocalAddr(sockfd));
    TcpConnSharedPtr conn(
        new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        boost::bind(&TcpServer::removeConnection, this, _1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnSharedPtr& conn)
{
    loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnSharedPtr& conn)
{
    loop_->assertInLoopThread();
    LOG << "TcpServer::removeConnection [" << name_
            << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1); (void)n;
    EventLoop* connLoop = conn->getLoop();
    connLoop->queueInLoop(
        boost::bind(&TcpConnection::connectDestroyed, conn));
}