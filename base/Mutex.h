#ifndef SERVER_MUTEX_H
#define SERVER_MUTEX_H

#include <pthread.h>
#include <boost/noncopyable.hpp>

#include "util.h"

namespace nut
{
class MutexLock
{
    public:
        MutexLock()
            //:pid_(0)
        { pthread_mutex_init(&mutex,NULL); }

        ~MutexLock()
        { pthread_mutex_destroy(&mutex); }

        void lock()
        { 
            pthread_mutex_lock(&mutex);
            //pid_ = currentThread::tid();
        }

        void unlock()
        {
            //pid_ = 0;
            pthread_mutex_unlock(&mutex);
        }

        // bool isLockedByThisThread()
        // {
        //     return pid_ == currentThread::tid();
        // }

        pthread_mutex_t* getMutex()
        {
            return &mutex;
        }
    private:
        //pid_t pid_;
        pthread_mutex_t mutex;
};

class MutexLockGuard
{
    public:
        MutexLockGuard(MutexLock& mutex)
        :mutexLock_(mutex)
        { mutexLock_.lock(); }

        ~MutexLockGuard()
        { mutexLock_.unlock();}
    private:

        MutexLock& mutexLock_;
    
};

#define MutexLockGuard(x) static_assert(false,"missing mutex var name")

}

#endif