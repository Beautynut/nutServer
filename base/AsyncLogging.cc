#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"
#include <iostream>

using namespace nut;

AsyncLogging::AsyncLogging(const std::string& filename)
    :running_(false),
    filename_(filename),
    mutex_(),
    cond_(mutex_),
    latch_(1),
    thread_(std::bind(&AsyncLogging::threadFunc,this)),
    buffers_(),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer)
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

// 将message 加入缓存,然后唤醒日志线程
void AsyncLogging::append(const char* message,int len)
{
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(message, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(message, len);
        cond_.notify();
  }
}

void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();
  Logfile output(filename_);
  uBufferPtr newBuffer1(new Buffer());
  uBufferPtr newBuffer2(new Buffer());
  newBuffer1->bzero();
  newBuffer2->bzero();
  uBufferPtrVector buffersToWrite;
  buffersToWrite.reserve(16);
  while(running_)
  {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());
    {
        MutexLockGuard lock(mutex_);
        // unusual usage!
        //   if (buffers_.empty())  // unusual usage!
        //   {
        //     cond_.waitForSeconds(flushInterval_);
        //   }
        // TODO:修改为定时等待
        while(buffers_.empty())
        {
            cond_.wait();
        }
        
        buffers_.push_back(std::move(currentBuffer_));
        currentBuffer_ = std::move(newBuffer1);
        buffersToWrite.swap(buffers_);
        if (!nextBuffer_)
        {
            nextBuffer_ = std::move(newBuffer2);
        }
    }

    assert(!buffersToWrite.empty());

    // 注释掉的这段代码的作用是日志消息太多丢弃一部分

    // if (buffersToWrite.size() > 25)
    // {
    //   char buf[256];
    //   snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
    //            Timestamp::now().toString().c_str(),
    //            buffersToWrite.size()-2);
    //   fputs(buf, stderr);
    //   output.append(buf, static_cast<int>(strlen(buf)));
    //   buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
    // }
    
    for (const auto& buffer : buffersToWrite)
    {
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2)
    {
      buffersToWrite.resize(2);
    }

    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}
