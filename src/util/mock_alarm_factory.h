//
// Created by Chengke Wong on 2019/11/5.
//

#ifndef SRC_UTIL_MOCK_ALARM_FACTORY_H
#define SRC_UTIL_MOCK_ALARM_FACTORY_H

#include "../base/alarm_factory.h"
#include "../base/time_base.h"
#include "../base/util.h"
#include <map>

class MockAlarmFactory : public AlarmFactory {
public:
  DISALLOW_COPY_AND_ASSIGN(MockAlarmFactory)

  MockAlarmFactory();
  ~MockAlarmFactory() override = default;

  Alarm *createAlarm(std::unique_ptr<Alarm::Delegate> delegate) override;

  TimeBase now() override;

  typedef std::multimap<TimeBase, Alarm *> AlarmMap;
  typedef AlarmMap::iterator AlarmToken;

  AlarmToken registerAlarm(Alarm *alarm);
  void unregisterAlarm(AlarmToken token);

  void elapse(TimeBase::Delta delta);

  bool runAlarmEvent();

private:
  AlarmMap alarm_map_;
  TimeBase now_;
};

#endif // SRC_UTIL_MOCK_ALARM_FACTORY_H
