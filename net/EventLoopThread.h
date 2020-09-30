#ifndef SERVER_EVENTLOOP_THREAD_H
#define SERVER_EVENTLOOP_THREAD_H

#include "../base/Thread.h"
#include "../base/Mutex.h"
#include "../base/Condition.h"

namespace nut
{
class EventLoop;
class EventLoopThread
{
    public:
        EventLoopThread();
        ~EventLoopThread();

        EventLoop* startLoop();
        void threadFunc();
    private:
        EventLoop* loop_;
        MutexLock mutex_;
        Condition cond_;
        Thread thread_;
};
}

#endif