//
// Created by Chengke Wong on 2019/10/4.
//

#include "epoll_alarm_factory.h"
#include "epoll_alarm.h"
#include <glog/logging.h>
#include <string.h>
#include <sys/time.h>

EpollAlarmFactory::EpollAlarmFactory(EpollServer *server) : server_(server) {
  // we must update the clock before call to now()
}

TimeBase EpollAlarmFactory::now() { return server_->now(); }

Alarm *
EpollAlarmFactory::createAlarm(std::unique_ptr<Alarm::Delegate> delegate) {
  return new EpollAlarm(server_, std::move(delegate));
}
