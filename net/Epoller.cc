#include "EventLoop.h"
#include "Channel.h"
#include "Epoller.h"

using namespace nut;

Epoller::Epoller(EventLoop* loop)
    :loop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    epollEvents_(16)
{
    if(epollfd_ < 0)
    {
        LOG<<"epoll_create error";
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeout, ChannelList* activechannels)
{
    int nCounts = epoll_wait(epollfd_,
                            epollEvents_.data(),
                            static_cast<int>(epollEvents_.size()),
                            timeout);
    Timestamp now(Timestamp::now());
    if(nCounts > 0)
    {
        fillActiveChannels(nCounts,activechannels);
        if(static_cast<int>(epollEvents_.size()) == nCounts)
        {
            epollEvents_.resize(nCounts*2);
        }
    }
    else if(nCounts == 0)
    {
        LOG<<"nothing happened";
    }
    else
    {
        LOG<<"epoll_wait failed";
    }
    return now;
    
}

void Epoller::fillActiveChannels(int nCounts, ChannelList* activeChannels)
{
    for(int i = 0;i< nCounts;i++)
    {
        Channel* channel = static_cast<Channel*>(epollEvents_[i].data.ptr);
        channel->setRevents(epollEvents_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoller::updateChannel(Channel* channel)
{
    int status = channel->index();
    // -1是新channel,1是已加入的channel,2是不关注任何事件的channel
    if(status == -1||status == 2)
    {
        int fd = channel->fd();
        if(status == -1)
        {
            assert(ChannelMap_.find(fd) == ChannelMap_.end());
            ChannelMap_[fd] = channel;
        }
        channel->set_index(1);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        if(channel->isNothingFocus())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(2);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }  
    }  
}

void Epoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    assert(ChannelMap_.find(fd) != ChannelMap_.end());
    assert(ChannelMap_[fd] == channel);
    assert(channel->isNothingFocus());
    channel->disableAll();
    size_t n = ChannelMap_.erase(fd);
    assert(n == 1);

    if(channel->index() == 1)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(-1);
}

void Epoller::update(int operation,Channel* channel)
{
    struct epoll_event epollEvent;
    bzero(&epollEvent,sizeof(epollEvent));
    epollEvent.events = channel->events();
    epollEvent.data.ptr = channel;
    int fd = channel->fd();

    if(::epoll_ctl(epollfd_, operation, fd, &epollEvent) < 0)
    {
        LOG<<"epoll_ctl error,op : "<< operation << "fd = "<<fd;
    }
}


