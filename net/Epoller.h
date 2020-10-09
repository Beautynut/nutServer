#ifndef SERVER_EPOLLER_H
#define SERVER_EPOLLER_H

#include <sys/epoll.h>
#include <vector>
#include <map>

#include "../base/Timestamp.h"
namespace nut
{
class EventLoop;
class Channel;
class Epoller
{
    public:
        typedef std::vector<Channel*> ChannelList;
        Epoller(EventLoop* loop);
        ~Epoller();

        Timestamp poll(int timeout,ChannelList* activeChannels);

        void fillActiveChannels(int nCounts, ChannelList* activeChannels);
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

        void update(int operation,Channel* channel);

    private:
    EventLoop* loop_;

    int epollfd_;
    std::vector<struct epoll_event> epollEvents_;

    std::map<int,Channel*> ChannelMap_;
};
}

#endif