#ifndef SERVER_POLLER_H
#define SERVER_POLLER_H

#include <map>
#include <vector>

#include "../base/Timestamp.h"
#include "EventLoop.h"

struct pollfd;

namespace nut
{

class Channel;

class Poller : boost::noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;

  EventLoop* ownerLoop_;
  PollFdList pollfds_;
  ChannelMap channels_;
};

}
#endif  // MUDUO_NET_POLLER_H