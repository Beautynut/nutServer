#include "net/EventLoop.h"
#include "net/Socket.h"
#include "net/TcpServer.h"
#include <stdio.h>

typedef boost::shared_ptr<nut::TcpConnection> TcpConnSharedPtr;
void onConnection(const TcpConnSharedPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): tid=%d new connection [%s] from %s\n",
           currentThread::tid(),
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
  }
  else
  {
    printf("onConnection(): tid=%d connection [%s] is down\n",
           currentThread::tid(),
           conn->name().c_str());
  }
}

void onMessage(const TcpConnSharedPtr& conn,
               nut::Buffer* buf,
               nut::Timestamp receiveTime)
{
  printf("onMessage(): tid=%d received %zd bytes from connection [%s] at\n",
         currentThread::tid(),
         buf->readableBytes(),
         conn->name().c_str()
         );
  std::string str = "HTTP/1.0 400 BAD REQUEST\r\n";
  str += "Content-type: text/html\r\n";
  str += "\r\n";
  str += "<P>Your browser sent a bad request, ";
  str += "such as a POST without a Content-Length.\r\n";
  conn->send(str);
  conn->shutdown();
  //printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  nut::inetAddr listenAddr(9981);
  nut::EventLoop loop;

  nut::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNums(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}