#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>

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


#endif