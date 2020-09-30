#ifndef SERVER_TIMER_H
#define SERVER_TIMER_H

#include "../base/Timestamp.h"

#include <boost/function.hpp>

namespace nut
{

class Timer
{
    public:
        typedef boost::function<void()> Callback;

        Timer(Timestamp when,const Callback& cb,double interval);

        void run() const { callback_(); }
        bool isRepeat() const { return repeat_; }
        Timestamp expiration() { return expirationDate_; }

        void restart(Timestamp now);
    private:
        // 过期时间
        Timestamp expirationDate_;
        const Callback callback_;
        // 时间间隔
        double interval_;
        bool repeat_;
    
};

class TimerId
{
    public:
        explicit TimerId(Timer* timer)
            : value_(timer)
        {}
    private:
        Timer* value_;
};

}

#endif