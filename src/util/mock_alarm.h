//
// Created by Chengke Wong on 2019/11/5.
//

#ifndef SRC_UTIL_MOCK_ALARM_H
#define SRC_UTIL_MOCK_ALARM_H

#include "mock_alarm_factory.h"

class MockAlarm : public Alarm {
public:
  DISALLOW_COPY_AND_ASSIGN(MockAlarm)
  explicit MockAlarm(MockAlarmFactory *factory,
                     std::unique_ptr<Delegate> delegate);

  ~MockAlarm() override;

private:
  void setImpl() override;

  void cancelImpl() override;

  MockAlarmFactory *factory_;
  MockAlarmFactory::AlarmToken token_;
};

#endif // SRC_UTIL_MOCK_ALARM_H
