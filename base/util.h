#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

//#include "Logging.h"

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <boost/implicit_cast.hpp>

namespace currentThread
{
    extern __thread int t_cachedTid;

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            if(t_cachedTid == 0)
            {
                t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            }
        }
        return t_cachedTid;
    }
}
namespace nut
{
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
void toHostPort(char* buf, size_t size,
                const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port,
                struct sockaddr_in* addr);
struct sockaddr_in getLocalAddr(int sockfd);
int getSocketError(int sockfd);                        
}


#endif