#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <boost/function.hpp>
#include <string>

#include "CountDownLatch.h"

namespace nut
{
class Thread :  
{
    public:
        typedef std::function<void()> ThreadFunc;
        explicit Thread(const ThreadFunc&);
        ~Thread();
        void start();
        int join();
        bool started() const { return started_; }
        pid_t tid() const { return tid_; }

    private:
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        CountDownLatch latch_;
};
}

#endif