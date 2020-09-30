#include "net/EventLoop.h"
#include "net/Socket.h"
#include <stdio.h>

void newConnection(int sockfd, const nut::inetAddr& peerAddr)
{
  printf("newConnection(): accepted a new connection from \n");
  ::write(sockfd, "How are you?\n", 13);
  ::close(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  nut::inetAddr listenAddr(9981);
  nut::EventLoop loop;

  nut::Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnection);
  acceptor.listen();

  loop.loop();
}