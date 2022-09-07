//
// Created by Chengke Wong on 2019/9/28.
//

#include "alarm.h"
#include <glog/logging.h>

Alarm::Alarm(std::unique_ptr<Delegate> delegate)
    : deadline_(TimeBase::zero()), delegate_(std::move(delegate)) {}

void Alarm::set(TimeBase new_deadline) {
  DCHECK(!isSet());
  DCHECK(new_deadline.isInitialized());
  deadline_ = new_deadline;
  setImpl();
}

void Alarm::cancel() {
  if (!isSet()) {
    return;
  }
  deadline_ = TimeBase::zero();
  cancelImpl();
}

bool Alarm::isSet() const { return deadline_.isInitialized(); }

void Alarm::fire() {
  if (!isSet()) {
    return;
  }
  deadline_ = TimeBase::zero();
  delegate_->onAlarm();
}

void Alarm::update(TimeBase new_deadline, TimeBase::Delta granularity) {
  if (!new_deadline.isInitialized()) {
    cancel();
    return;
  }

  if (std::abs((new_deadline - deadline_).toMicroseconds()) <
      granularity.toMicroseconds()) {
    return;
  }
  const bool was_set = isSet();
  deadline_ = new_deadline;
  if (was_set) {
    updateImpl();
  } else {
    setImpl();
  }
}

void Alarm::updateImpl() {
  const TimeBase new_deadline = deadline_;
  deadline_ = TimeBase::zero();
  cancelImpl();
  deadline_ = new_deadline;
  setImpl();
}
