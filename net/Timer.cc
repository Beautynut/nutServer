#include "Timer.h"

using namespace nut;

Timer::Timer(Timestamp when,const Callback& cb,double interval)
    :expirationDate_(when),
     callback_(cb),
     interval_(interval),
     repeat_( interval_ > 0.0)
{
}

/** 
 * @brief 将到期时间修改为从现在开始往后interval_秒
 * @param now gettimeofday()
 *
 */
void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expirationDate_ = addTime(now, interval_);
  }
  else
  {
    expirationDate_ = Timestamp::invalid();
  }
}