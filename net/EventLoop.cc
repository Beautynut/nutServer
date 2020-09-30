#include <boost/bind.hpp>
#include <sys/eventfd.h>
#include <assert.h>
#include <poll.h>
#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace nut;

__thread EventLoop* t_loopInThisThread = 0;

static int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
      LOG << "Failed in create eventfd";
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
    quit_(false),
    threadId_(currentThread::tid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
      LOG << "Another EventLoop " << t_loopInThisThread
                << " exists in this thread " << threadId_;
    }
    else
    {
      t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

// 一个线程只能有一个eventloop,如果是a线程调用了b线程的loop,会因断言而失败
void EventLoop::assertInLoopThread()
{
    assert(isInLoopThread());
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
      activeChannels_.clear();
      poller_->poll(10000,&activeChannels_);
      for(ChannelList::iterator it = activeChannels_.begin();
        it != activeChannels_.end();++it)
      {
          (*it)->handler();
      }
      doPendingFunctors();
    }

    LOG << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel)
{
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeUp();
    }
}

TimerId EventLoop::RunOnTime(const Timestamp& time, const Callback& cb)
{
    return timerQueue_->addTimer(time, cb, 0.0);
}

TimerId EventLoop::RunAfterTime(double delay, const Callback& cb)
{
    Timestamp time(addTime(Timestamp::now(),delay));
    return RunOnTime(time, cb);
}

TimerId EventLoop::RunEachInterval(double interval, const Callback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(time, cb, interval);
}

void EventLoop::runInLoop(const Function& func)
{
    if(isInLoopThread())
    {
      func();
    }
    else
    {
      queueInLoop(func);
    }  
}

void EventLoop::wakeUp()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
      LOG << "EventLoop::wakeUp() writes " << n << " bytes instead of 8";
    }

}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
      LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

/** 
 * @brief 将func加入EventLoop的任务队列中
 * @param func    需要执行的回调
 */
void EventLoop::queueInLoop(const Function& func)
{
    {
      MutexLockGuard lock(mutex_);
      pendingFunctors_.push_back(func);
    }

    // 这里需要唤醒的情况有两种
    // 一种是其他线程将任务加入当前线程的时候,则需要唤醒当前线程
    // 另一种情况是在当前线程执行PendingFunctors时,所执行的回调调用了queueInLoop,也应该唤醒当前线程
    // 即通过向wakeupFd_写入数据的方式,唤醒线程,避免当前线程阻塞在this->loop()中的poll()或epoll_wait()处
    if(!isInLoopThread() || runingPendingFunctors_)
    {
      wakeUp();
    }
}

/** 
 * @brief 执行EventLoop的任务队列中
 */
void EventLoop::doPendingFunctors()
{
    std::vector<Function> tempFunctors;
    runingPendingFunctors_ = true;

    // 这里将pendingFunctors和临时的tempFunctors进行交换,减少锁的争用,缩小临界区
    {
        MutexLockGuard lock(mutex_);
        tempFunctors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < tempFunctors.size(); i++)
    {
        tempFunctors[i]();
    }
    runingPendingFunctors_ = false;
    
}

