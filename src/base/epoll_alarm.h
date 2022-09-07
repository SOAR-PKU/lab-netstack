//
// Created by Chengke Wong on 2019/10/4.
//

#ifndef SRC_BASE_EPOLL_ALARM_H
#define SRC_BASE_EPOLL_ALARM_H

#include "epoll_server.h"

/**
 * @brief EpollAlarm is the alarm correspond to epoll events.
 * All EpollAlarms are managed by EpollServer
 */
class EpollAlarm : public Alarm {
public:
  explicit EpollAlarm(EpollServer *epoll_server,
                      std::unique_ptr<Delegate> delegate);

  ~EpollAlarm() override;

private:
  void setImpl() override;

  void cancelImpl() override;

  EpollServer *server_;
  EpollServer::AlarmToken token_;
};

#endif // SRC_BASE_EPOLL_ALARM_H
