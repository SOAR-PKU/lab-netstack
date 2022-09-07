//
// Created by Chengke Wong on 2019/9/28.
//

#include "time_base.h"
#include <sstream>

std::string TimeBase::Delta::toDebugValue() const {
  const int64_t one_ms = 1000;
  const int64_t one_s = 1000 * one_ms;

  int64_t absolute_value = std::abs(time_offset_);
  std::stringstream fmt;
  if (absolute_value > one_s && absolute_value % one_s == 0) {
    fmt << "s" << time_offset_ / one_s;
  }
  if (absolute_value > one_ms && absolute_value % one_ms == 0) {
    fmt << "ms" << time_offset_ / one_ms;
  }
  fmt << "us" << time_offset_;
  return fmt.str();
}