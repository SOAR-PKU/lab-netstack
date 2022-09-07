//
// Created by Chengke Wong on 2019/9/28.
//

#ifndef SRC_BASE_ALARM_H
#define SRC_BASE_ALARM_H

#include "time_base.h"
#include "util.h"

/**
 * @brief Alarm base class.
 * When alarm deadline is reached, it dispatches its delegate to perform
 * actions.
 *
 */
class Alarm {
public:
  /**
   * @brief The delegate
   *
   */
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void onAlarm() = 0;
  };

  DISALLOW_COPY_AND_ASSIGN(Alarm)
  explicit Alarm(std::unique_ptr<Delegate> delegate);
  virtual ~Alarm() = default;

  /**
   * @brief Sets the alarm to fire at |deadline|.  MUST NOT be called while
   * the alarm is set.  To reschedule an alarm, call cancel() first,
   * then set().
   */
  void set(TimeBase new_deadline);
  void
  update(TimeBase new_deadline,
         TimeBase::Delta granularity = TimeBase::Delta::fromMilliseconds(1));
  void cancel();
  bool isSet() const;

  inline TimeBase deadline() const { return deadline_; }
  void fire();

protected:
  /**
   * @brief Actions that are performed when setting/canceling/updating an alarm.
   *
   */
  virtual void setImpl() = 0;
  virtual void cancelImpl() = 0;
  virtual void updateImpl();

private:
  TimeBase deadline_;
  std::unique_ptr<Delegate> delegate_;
};

/**
 * @brief Macro utility for defining an alarm delegate.
 * A typical use is to set `class_base` to the class itself
 * when defining a delegate in a class and set `action` to
 * a method of the class.
 */
#define DEFINE_ALARM_DELEGATE(name, class_base, action)                        \
  class name : public Alarm::Delegate {                                        \
  public:                                                                      \
    explicit name(class_base *a) : class_(a) {}                                \
    DISALLOW_COPY_AND_ASSIGN(name)                                             \
    void onAlarm() override { class_->action(); }                              \
                                                                               \
  private:                                                                     \
    class_base *class_;                                                        \
  }

#endif // SRC_BASE_ALARM_H
