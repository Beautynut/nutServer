#include "EventLoopThread.h"
#include "EventLoop.h"

#include <boost/bind.hpp>
using namespace nut;

EventLoopThread::EventLoopThread()
    :loop_(NULL),
     mutex_(),
     cond_(mutex_),
     thread_(boost::bind(&EventLoopThread::threadFunc,this))
    {

    }

EventLoopThread::~EventLoopThread()
{

}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();

    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait();
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
}