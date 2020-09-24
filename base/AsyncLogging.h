#ifndef SERVER_ASYNCLOGGING_H
#define SERVER_ASYNCLOGGING_H

#include <vector>
#include <string>
#include <atomic>

#include "Logging.h"
#include "Thread.h"
#include "CountDownLatch.h"

namespace nut
{
class AsyncLogging
{
    public:
        AsyncLogging(const std::string& filename);
        ~AsyncLogging()
        {
            if(running_)
            {
                stop();
            }
        }

        void append(const char* message,int len);

        void start()
        {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop()
        {
            running_ = false;
            cond_.notify();
            thread_.join();
        }

    private:
        void threadFunc();


        // Buffer
        typedef FixedBuffer<LargeBuffer> Buffer;
        typedef std::vector<std::unique_ptr<Buffer>> uBufferPtrVector;
        typedef uBufferPtrVector::value_type uBufferPtr;

        std::atomic<bool> running_;
        const std::string filename_;

        MutexLock mutex_;
        Condition cond_;
        CountDownLatch latch_;

        Thread thread_;
        uBufferPtrVector buffers_;
        uBufferPtr currentBuffer_;
        uBufferPtr nextBuffer_;

};
}


#endif