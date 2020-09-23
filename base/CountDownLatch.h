#ifndef COUNT_DOWN_LATCH_H
#define COUNT_DOWN_LATCH_H

#include "Condition.h"
#include "Mutex.h"

namespace nut
{
class CountDownLatch
{
    public:

        explicit CountDownLatch(int count);
        ~CountDownLatch();

        void wait();
        void countDown();
        int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}

#endif