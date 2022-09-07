//
// Created by Chengke Wong on 2019/9/28.
//

#ifndef SRC_BASE_TIME_BASE_H
#define SRC_BASE_TIME_BASE_H

#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>

/**
 * @brief A Time is a purely relative time.
 * Time values from different clocks cannot be compared to each other.
 * This virtual time gives us more flexibility on testing.
 */
class TimeBase {
public:
  explicit constexpr TimeBase(int64_t time) : time_(time) {}

  // A Time::Delta represents the signed difference between two points in
  // time, stored in microsecond resolution.
  class Delta {
  public:
    // Create a object with an offset of 0.
    static constexpr Delta zero() { return Delta(0); }

    // Create a object with infinite offset time.
    static constexpr Delta infinite() { return Delta(kInfiniteTimeUs); }

    // Converts a number of seconds to a time offset.
    static constexpr Delta fromSeconds(int64_t secs) {
      return Delta(secs * 1000 * 1000);
    }

    // Converts a number of milliseconds to a time offset.
    static constexpr Delta fromMilliseconds(int64_t ms) {
      return Delta(ms * 1000);
    }

    // Converts a number of microseconds to a time offset.
    static constexpr Delta fromMicroseconds(int64_t us) { return Delta(us); }

    // Converts the time offset to a rounded number of seconds.
    inline int64_t toSeconds() const { return time_offset_ / 1000 / 1000; }

    // Converts the time offset to a rounded number of milliseconds.
    inline int64_t toMilliseconds() const { return time_offset_ / 1000; }

    // Converts the time offset to a rounded number of microseconds.
    inline int64_t toMicroseconds() const { return time_offset_; }

    inline struct timeval toTimeval() const {
      return timeval{time_offset_ / 1000000, time_offset_ % 1000000};
    }

    inline bool isZero() const { return time_offset_ == 0; }

    inline bool isInfinite() const { return time_offset_ == kInfiniteTimeUs; }

    std::string toDebugValue() const;

    static const int64_t kInfiniteTimeUs = std::numeric_limits<int64_t>::max();

  private:
    friend inline bool operator==(TimeBase::Delta lhs, TimeBase::Delta rhs);

    friend inline bool operator<(TimeBase::Delta lhs, TimeBase::Delta rhs);

    friend inline TimeBase::Delta operator<<(TimeBase::Delta lhs, size_t rhs);

    friend inline TimeBase::Delta operator>>(TimeBase::Delta lhs, size_t rhs);

    friend inline TimeBase::Delta operator+(TimeBase::Delta lhs,
                                            TimeBase::Delta rhs);

    friend inline TimeBase::Delta operator-(TimeBase::Delta lhs,
                                            TimeBase::Delta rhs);

    friend inline TimeBase::Delta operator*(TimeBase::Delta lhs, int rhs);

    friend inline TimeBase::Delta operator*(TimeBase::Delta lhs, double rhs);

    friend inline TimeBase operator+(TimeBase lhs, TimeBase::Delta rhs);

    friend inline TimeBase operator-(TimeBase lhs, TimeBase::Delta rhs);

    friend inline TimeBase::Delta operator-(TimeBase lhs, TimeBase rhs);

    explicit constexpr Delta(int64_t time_offset) : time_offset_(time_offset) {}

    int64_t time_offset_;
  };

  // Creates a new Time with an internal value of 0.  IsInitialized()
  // will return false for these times.
  static constexpr TimeBase zero() { return TimeBase(0); }

  // Creates a new Time with an infinite time.
  static constexpr TimeBase infinite() {
    return TimeBase(Delta::kInfiniteTimeUs);
  }

  TimeBase(const TimeBase &other) = default;

  TimeBase &operator=(const TimeBase &other) = default;

  // Produce the internal value to be used when logging.  This value
  // represents the number of microseconds since some epoch.  It may
  // be the UNIX epoch on some platforms.  On others, it may
  // be a CPU ticks based value.
  inline int64_t toDebuggingValue() const { return time_; }

  inline bool isInitialized() const { return 0 != time_; }

private:
  friend inline bool operator==(TimeBase lhs, TimeBase rhs);

  friend inline bool operator<(TimeBase lhs, TimeBase rhs);

  friend inline TimeBase operator+(TimeBase lhs, TimeBase::Delta rhs);

  friend inline TimeBase operator-(TimeBase lhs, TimeBase::Delta rhs);

  friend inline TimeBase::Delta operator-(TimeBase lhs, TimeBase rhs);

  int64_t time_;
};

// Non-member relational operators for Time::Delta.
inline bool operator==(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return lhs.time_offset_ == rhs.time_offset_;
}
inline bool operator!=(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return !(lhs == rhs);
}
inline bool operator<(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return lhs.time_offset_ < rhs.time_offset_;
}
inline bool operator>(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return rhs < lhs;
}
inline bool operator<=(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return !(lhs < rhs);
}
inline TimeBase::Delta operator<<(TimeBase::Delta lhs, size_t rhs) {
  return TimeBase::Delta(lhs.time_offset_ << rhs);
}
inline TimeBase::Delta operator>>(TimeBase::Delta lhs, size_t rhs) {
  return TimeBase::Delta(lhs.time_offset_ >> rhs);
}

// Non-member relational operators for Time.
inline bool operator==(TimeBase lhs, TimeBase rhs) {
  return lhs.time_ == rhs.time_;
}
inline bool operator!=(TimeBase lhs, TimeBase rhs) { return !(lhs == rhs); }
inline bool operator<(TimeBase lhs, TimeBase rhs) {
  return lhs.time_ < rhs.time_;
}
inline bool operator>(TimeBase lhs, TimeBase rhs) { return rhs < lhs; }
inline bool operator<=(TimeBase lhs, TimeBase rhs) { return !(rhs < lhs); }
inline bool operator>=(TimeBase lhs, TimeBase rhs) { return !(lhs < rhs); }

// Override stream output operator for gtest or CHECK macros.
inline std::ostream &operator<<(std::ostream &output, const TimeBase t) {
  output << t.toDebuggingValue();
  return output;
}

// Non-member arithmetic operators for Time::Delta.
inline TimeBase::Delta operator+(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return TimeBase::Delta(lhs.time_offset_ + rhs.time_offset_);
}
inline TimeBase::Delta operator-(TimeBase::Delta lhs, TimeBase::Delta rhs) {
  return TimeBase::Delta(lhs.time_offset_ - rhs.time_offset_);
}
inline TimeBase::Delta operator*(TimeBase::Delta lhs, int rhs) {
  return TimeBase::Delta(lhs.time_offset_ * rhs);
}
inline TimeBase::Delta operator*(TimeBase::Delta lhs, double rhs) {
  return TimeBase::Delta(
      static_cast<int64_t>(std::llround(lhs.time_offset_ * rhs)));
}
inline TimeBase::Delta operator*(int lhs, TimeBase::Delta rhs) {
  return rhs * lhs;
}
inline TimeBase::Delta operator*(double lhs, TimeBase::Delta rhs) {
  return rhs * lhs;
}

// Non-member arithmetic operators for Time and Time::Delta.
inline TimeBase operator+(TimeBase lhs, TimeBase::Delta rhs) {
  return TimeBase(lhs.time_ + rhs.time_offset_);
}
inline TimeBase operator-(TimeBase lhs, TimeBase::Delta rhs) {
  return TimeBase(lhs.time_ - rhs.time_offset_);
}
inline TimeBase::Delta operator-(TimeBase lhs, TimeBase rhs) {
  return TimeBase::Delta(lhs.time_ - rhs.time_);
}

// Override stream output operator for gtest.
inline std::ostream &operator<<(std::ostream &output,
                                const TimeBase::Delta delta) {
  output << delta.toDebugValue();
  return output;
}

#endif // SRC_BASE_TIME_BASE_H
