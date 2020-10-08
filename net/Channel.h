#ifndef SERVER_CHANNEL_H
#define SERVER_CHANNEL_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include "../base/Timestamp.h"

namespace nut
{
class EventLoop;
class Channel
{
    public:
        typedef boost::function<void()> Callback;
        typedef boost::function<void(Timestamp)> ReadCallback;

        Channel(EventLoop* loop,int fd);
        ~Channel();

        void handler(Timestamp recvTime);
        void setReadCallback(const ReadCallback& cb){ readCallback_ = cb; }
        void setWriteCallback(const Callback& cb){ writeCallback_ = cb; }
        void setErrorCallback(const Callback& cb){ errorCallback_ = cb; }
        void setCloseCallback(const Callback& cb){ closeCallback_ = cb; }

        void enableReading() { events_ |= readEvent_; update(); }
        void enableWriting() { events_ |= writeEvent_; update(); }
        void disableWriting() { events_ &= ~writeEvent_; update(); }
        void disableAll() { events_ = noneEvent_; update(); }
        bool isWriting() const { return events_ & writeEvent_; }

        int fd() const { return fd_; }
        int index() const { return index_; }
        int events() const { return events_; }
        void setRevents(int revt) { revents_ = revt; }
        void set_index(int idx) { index_ = idx; }
        bool isNothingHappened() const { return events_ == noneEvent_; }

        EventLoop* ownerLoop(){ return loop_; }

    private:
        void update();

        static const int noneEvent_;
        static const int readEvent_;
        static const int writeEvent_;

        EventLoop* loop_;
        const int fd_;
        
        // events_是epoller需要关注的事件,epoller会用channel.events()构建 pollfd
        // revents_则是接收到的事件
        int events_;
        int revents_;
        int index_;

        ReadCallback readCallback_;
        Callback writeCallback_;
        Callback errorCallback_;
        Callback closeCallback_;

        bool eventHandling_;

};
}

#endif