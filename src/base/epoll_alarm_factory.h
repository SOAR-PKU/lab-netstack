//
// Created by Chengke Wong on 2019/10/4.
//

#ifndef SRC_BASE_EPOLL_ALARM_FACTORY_H
#define SRC_BASE_EPOLL_ALARM_FACTORY_H

#include "epoll_server.h"

class EpollAlarmFactory : public AlarmFactory {
public:
  explicit EpollAlarmFactory(EpollServer *server);
  ~EpollAlarmFactory() override = default;

  TimeBase now() override;

  Alarm *createAlarm(std::unique_ptr<Alarm::Delegate> delegate) override;

private:
  EpollServer *server_;
};

#endif // SRC_BASE_EPOLL_ALARM_FACTORY_H
