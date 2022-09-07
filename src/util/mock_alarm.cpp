//
// Created by Chengke Wong on 2019/11/5.
//

#include "mock_alarm.h"

MockAlarm::MockAlarm(MockAlarmFactory *factory,
                     std::unique_ptr<Delegate> delegate)
    : Alarm(std::move(delegate)), factory_(factory) {}

void MockAlarm::setImpl() { token_ = factory_->registerAlarm(this); }

void MockAlarm::cancelImpl() { factory_->unregisterAlarm(token_); }

MockAlarm::~MockAlarm() { cancel(); }
