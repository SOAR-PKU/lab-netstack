//
// Created by Chengke Wong on 2019/9/30.
//

#include "epoll_server.h"
#include <cstring>
#include <glog/logging.h>
#include <sys/time.h>

EpollServer::EpollServer() : epfd_(-1), cb_map_(), events_(), now_(0) {

  // Since Linux 2.6.8,
  //       the size argument is ignored, but must be greater than zero;
  epfd_ = epoll_create(1);
  if (epfd_ == -1) {
    LOG(FATAL) << "epoll_create: " << strerror(errno);
    return;
  }

  updateNow();
}

/**
 * @brief Register a file descriptor to EpollServer
 * so that it can be noticed by epoll
 *
 * @param fd
 * @param cb
 */
bool EpollServer::registerRead(int fd, EpollCallback *cb) {
  auto iter = cb_map_.find(fd);
  if (iter != cb_map_.end()) {
    iter->second = cb;
    return true;
  }

  cb_map_[fd] = cb;

  struct epoll_event event = {};
  event.events = EPOLLIN | EPOLLERR;
  event.data.fd = fd;
  int rv;
  rv = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
  if (rv < 0) {
    LOG(ERROR) << strerror(errno);
    return false;
  }

  return true;
}

/**
 * @brief Run non-blocking IO loops.
 * It first waits for incoming frames (read events) for a certain period of time
 * and then fires all alarms whose deadline is elapsed.
 */
bool EpollServer::runEventLoop(TimeBase::Delta wait) {
  bool rv = false;
  updateNow();
  rv |= runReadEvent(std::min(wait, incomingAlarm()));
  rv |= runAlarmEvent();
  return rv;
}

bool EpollServer::runReadEvent(const TimeBase::Delta &wait) {
  int rv;

  rv = epoll_wait(epfd_, events_, events_size_, wait.toMilliseconds());
  if (rv < 0) {
    LOG(FATAL) << "epoll_wait failed";
    return false;
  }

  if (rv == 0) {
    return false;
  }

  for (int i = 0; i < rv; i++) {
    int fd = events_[i].data.fd;
    if (events_[i].events & EPOLLERR) {
      DLOG(INFO) << "fd: " << fd << " error";
    }

    auto iter = cb_map_.find(fd);
    if (iter != cb_map_.end()) {
      iter->second->onReadable();
    }
  }
  return true;
}

EpollServer::AlarmToken EpollServer::registerAlarm(Alarm *alarm) {
  return alarm_map_.emplace(alarm->deadline(), alarm);
}

void EpollServer::unregisterAlarm(AlarmToken token) { alarm_map_.erase(token); }

bool EpollServer::runAlarmEvent() {
  while (!alarm_map_.empty() && now() >= alarm_map_.begin()->first) {
    auto alarm = alarm_map_.begin();
    alarm->second->fire();
    alarm_map_.erase(alarm);
  }
  return false;
}

void EpollServer::updateNow() {
  timeval time{};
  int rv = gettimeofday(&time, nullptr);
  if (rv < 0) {
    LOG(FATAL) << "gettimeofday: " << strerror(errno);
    return;
  }

  int64_t t = time.tv_usec / 1000;
  t += time.tv_sec * 1000 * 1000;
  now_ = TimeBase(t);
}

const TimeBase &EpollServer::now() const { return now_; }

TimeBase::Delta EpollServer::incomingAlarm() {
  TimeBase incoming = alarm_map_.begin()->first;
  TimeBase::Delta rv = incoming - now();
  if (rv < TimeBase::Delta::zero()) {
    return TimeBase::Delta::zero();
  }
  return rv;
}
