//
// Created by Chengke Wong on 2019/9/30.
//

#ifndef SRC_BASE_EPOLL_SERVER_H
#define SRC_BASE_EPOLL_SERVER_H

#include "alarm_factory.h"
#include "time_base.h"
#include "util.h"
#include <map>
#include <sys/epoll.h>
#include <unordered_map>

/**
 * @brief Epoll IO callback for handing packets to upper layers.
 * Link layer should derive and implement this callback interface.
 *
 */
class EpollCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(EpollCallback)
  EpollCallback() = default;
  ~EpollCallback() = default;
  virtual void onReadable() = 0;
};

/**
 * @brief EpollServer handles all epoll events and manages all EpollAlarms.
 * @see runEventLoop
 *
 */
class EpollServer {
public:
  DISALLOW_COPY_AND_ASSIGN(EpollServer)
  EpollServer();

  bool registerRead(int fd, EpollCallback *cb);
  bool runEventLoop(TimeBase::Delta wait);

  typedef std::multimap<TimeBase, Alarm *> AlarmMap;
  typedef AlarmMap::iterator AlarmToken;

  AlarmToken registerAlarm(Alarm *alarm);
  void unregisterAlarm(AlarmToken token);

  const TimeBase &now() const;

private:
  int epfd_;
  std::unordered_map<int, EpollCallback *> cb_map_;
  static const int events_size_ = 256;
  struct epoll_event events_[256];
  AlarmMap alarm_map_;

  bool runReadEvent(const TimeBase::Delta &wait);
  bool runAlarmEvent();

  TimeBase::Delta incomingAlarm();

  void updateNow();
  TimeBase now_;
};

#endif // SRC_BASE_EPOLL_SERVER_H
