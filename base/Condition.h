#ifndef SERVER_CONDITION_H
#define SERVER_CONDITION_H

#include <pthread.h>

#include "Mutex.h"

namespace nut
{
    class Condition
    {
        public:
            Condition(MutexLock& mutex)
                :mutex_(mutex)
            { pthread_cond_init(&cond_,NULL); }
            
            ~Condition()
            { pthread_cond_destroy(&cond_); }

            void wait()
            { pthread_cond_wait(&cond_,mutex_.getMutex()); }
            void notify()
            { pthread_cond_signal(&cond_); }
            void notifyAll()
            { pthread_cond_broadcast(&cond_); }


        private:
            MutexLock& mutex_;
            pthread_cond_t cond_;
    };
}

#endif