#ifndef SERVER_EVENTLOOP_H
#define SERVER_EVENTLOOP_H

#include "../base/Thread.h"
#include "../base/util.h"
#include "../base/Timestamp.h"
#include "../base/Logging.h"
#include "TimerQueue.h"

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
namespace nut
{
class Channel;
class Epoller;
class EventLoop
{
    public:
        typedef std::vector<Channel*> ChannelList;
        typedef boost::function<void()> Callback;
        typedef boost::function<void()> Function;
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        void assertInLoopThread();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        void runInLoop(const Function& func);
        void queueInLoop(const Function& func);
        void wakeUp();
        void handleRead();
        void doPendingFunctors();

        bool isInLoopThread() const { return threadId_ == currentThread::tid(); }
        TimerId RunOnTime(const Timestamp& time, const Callback& cb);
        TimerId RunAfterTime(double delay, const Callback& cb);
        TimerId RunEachInterval(double interval, const Callback& cb);


    private:

        bool looping_;
        bool quit_;
        bool runingPendingFunctors_;
        const pid_t threadId_;
        ChannelList activeChannels_;
        boost::scoped_ptr<Epoller>epoller_;
        boost::scoped_ptr<TimerQueue> timerQueue_;
        int wakeupFd_;
        boost::scoped_ptr<Channel> wakeupChannel_;

        MutexLock mutex_;
        // 因为pendingFunctors_会暴露给其他线程,所以使用时必须加锁,后期考虑加入GUARD_BY(mutex_)防止忘记加锁
        std::vector<Function> pendingFunctors_;
};
}

#endif