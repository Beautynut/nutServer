#ifndef SERVER_TIMER_QUEUE_H
#define SERVER_TIMER_QUEUE_H

#include <set>
#include <vector>
#include "../base/Timestamp.h"
#include "Timer.h"
#include "Channel.h"

namespace nut
{
class EventLoop;
class TimerQueue
{
    public:
        typedef std::pair<Timestamp,Timer*> TimerIndex;
        typedef std::set<TimerIndex> TimerList;
        typedef boost::function<void()> Callback;

        TimerQueue(EventLoop* loop);
        ~TimerQueue();

        TimerId addTimer(Timestamp when,
                        const Callback& cb,
                        double interval);
        void addTimerInLoop(Timer* timer);
        
    private:
        void handler();
        std::vector<TimerIndex> getExpired(Timestamp now);
        void reset(const std::vector<TimerIndex>& expired,Timestamp now);

        bool insert(Timer* Timer);

        EventLoop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        TimerList timerLists_;
};
}

#endif