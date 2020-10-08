#include "Poller.h"
#include "Channel.h"
#include "../base/Logging.h"

#include <boost/implicit_cast.hpp>
#include <assert.h>
#include <poll.h>

using namespace nut;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop)
{
}

Poller::~Poller()
{
}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
  Timestamp now(Timestamp::now());
  if (numEvents > 0) {
    LOG << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOG << " nothing happended";
  } else {
    LOG << "Poller::poll()";
  }
  return now;
}

void Poller::fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = pollfds_.begin();
      pfd != pollfds_.end() && numEvents > 0; ++pfd)
  {
    if (pfd->revents > 0)
    {
      --numEvents;
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->setRevents(pfd->revents);
      activeChannels->push_back(channel);
    }
  }
}

void Poller::updateChannel(Channel* channel)
{
  assertInLoopThread();
  LOG << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size())-1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNothingHappened()) {
      // ignore this pollfd
      pfd.fd = -channel->fd()-1;
    }
  }
}

void Poller::removeChannel(Channel* channel)
{
  assertInLoopThread();
  LOG << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1); (void)n;
  // 这里从pollfds_中删除的时间复杂度是O(1)
  // 如果要删除的是最后一个元素 则直接删除
  // 如果不是,则将要删除的元素跟最后一个交换,然后删除
  if (boost::implicit_cast<size_t>(idx) == pollfds_.size()-1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd-1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}