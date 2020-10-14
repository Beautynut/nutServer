#include "TimerQueue.h"
#include "EventLoop.h"
#include "../base/Logging.h"

#include <sys/timerfd.h>
#include <boost/bind.hpp>

using namespace nut;

int createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC,
                                TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0)
    {
        LOG<<"create timerfd failed";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSeconds);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSeconds) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
        LOG << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    LOG << "timerfd_settime()";
  }
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timerLists_()
{
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::handler, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timerLists_.begin();
      it != timerLists_.end(); ++it)
  {
    delete it->second;
  }
}

TimerId TimerQueue::addTimer(Timestamp when,
                             const Callback& cb,
                             double interval)
{
  Timer* timer = new Timer(when, cb, interval);
  loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop,this,timer));
  return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
  bool isEarliestTimer = insert(timer);

  if (isEarliestTimer)
  {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::handler()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);

  std::vector<TimerIndex> expired = getExpired(now);

  // safe to callback outside critical section
  for (std::vector<TimerIndex>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    it->second->run();
  }

  reset(expired, now);
}

std::vector<TimerQueue::TimerIndex> TimerQueue::getExpired(Timestamp now)
{
  std::vector<TimerIndex> expired;
  TimerIndex sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator it = timerLists_.lower_bound(sentry);
  assert(it == timerLists_.end() || now < it->first);
  std::copy(timerLists_.begin(), it, back_inserter(expired));
  timerLists_.erase(timerLists_.begin(), it);

  return expired;
}

void TimerQueue::reset(const std::vector<TimerIndex>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for (std::vector<TimerIndex>::const_iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    if (it->second->isRepeat())
    {
      it->second->restart(now);
      insert(it->second);
    }
    else
    {
      // FIXME move to a free list
      delete it->second;
    }
  }

  if (!timerLists_.empty())
  {
    nextExpire = timerLists_.begin()->second->expiration();
  }

  if (nextExpire.valid())
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer* timer)
{
  bool isEarliestTimer = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timerLists_.begin();
  if (it == timerLists_.end() || when < it->first)
  {
    isEarliestTimer = true;
  }
  std::pair<TimerList::iterator, bool> result =
          timerLists_.insert(std::make_pair(when, timer));
  assert(result.second);
  return isEarliestTimer;
}



