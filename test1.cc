#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/HttpServer.h"

using namespace nut;
int main()
{
    EventLoop loop;
    HttpServer server(&loop,inetAddr(9981));

    server.start();
    loop.loop();
}