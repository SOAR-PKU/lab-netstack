//
// Created by Chengke Wong on 2019/9/28.
//

#ifndef SRC_BASE_ALARM_FACTORY_H
#define SRC_BASE_ALARM_FACTORY_H

#include "alarm.h"

/**
 * @brief AlarmFactory is responsable for creating all alarms.
 * This is a base class and derived classes should be defined to
 * create more specific and customized alarms.
 */
class AlarmFactory {
public:
  virtual ~AlarmFactory() = default;
  virtual Alarm *createAlarm(std::unique_ptr<Alarm::Delegate> delegate) = 0;
  virtual TimeBase now() = 0;
};

#endif // SRC_BASE_ALARM_FACTORY_H
