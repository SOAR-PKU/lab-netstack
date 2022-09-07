//
// Created by Chengke Wong on 2019/11/5.
//

#include "mock_alarm_factory.h"
#include "mock_alarm.h"
#include "gtest/gtest.h"

Alarm *
MockAlarmFactory::createAlarm(std::unique_ptr<Alarm::Delegate> delegate) {
  return new MockAlarm(this, std::move(delegate));
}

TimeBase MockAlarmFactory::now() { return now_; }

MockAlarmFactory::AlarmToken MockAlarmFactory::registerAlarm(Alarm *alarm) {
  EXPECT_TRUE(alarm->deadline() >= now_);
  return alarm_map_.emplace(alarm->deadline(), alarm);
}

void MockAlarmFactory::unregisterAlarm(AlarmToken token) {
  alarm_map_.erase(token);
}

MockAlarmFactory::MockAlarmFactory() : now_(0) {}

void MockAlarmFactory::elapse(TimeBase::Delta delta) {
  while (delta > TimeBase::Delta::zero()) {
    TimeBase::Delta step =
        alarm_map_.empty() ? delta
                           : std::min(delta, alarm_map_.begin()->first - now());
    now_ = now_ + step;
    delta = delta - step;
    runAlarmEvent();
  }
}

bool MockAlarmFactory::runAlarmEvent() {
  while (!alarm_map_.empty() && now() >= alarm_map_.begin()->first) {
    auto alarm = alarm_map_.begin();
    DLOG(INFO) << "fire alarm at " << now();
    alarm->second->fire();
    alarm_map_.erase(alarm);
  }
  return false;
}
