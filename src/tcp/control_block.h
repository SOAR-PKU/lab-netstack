//
// Created by Chengke on 2022/09/03
//

#ifndef TCPSTACK_TCB_H
#define TCPSTACK_TCB_H

#include "type.h"

#if defined(MODULO_COMPARISIONS) || defined(ALL_MODULO_COMPARISION)
#error
#endif

#define MODULO_COMPARISIONS(name, opt)                                         \
  inline bool name(SequenceNumber a, SequenceNumber b) {                       \
    return a - initial opt b - initial;                                        \
  }
#define ALL_MODULO_COMPARISION                                                 \
  MODULO_COMPARISIONS(equal, ==)                                               \
  MODULO_COMPARISIONS(lessThan, <)                                             \
  MODULO_COMPARISIONS(lessOrEqual, <=)                                         \
  MODULO_COMPARISIONS(greaterThan, >)                                          \
  MODULO_COMPARISIONS(greaterOrEqual, >=)                                      \
  inline bool inMiddle(SequenceNumber x, SequenceNumber a, SequenceNumber b) { \
    return lessOrEqual(a, x) && lessThan(x, b);                                \
  }

/**
 * @brief Transmission Control Block (TCB)
 * @see RFC 793 section 3.2
 */
struct ControlBlock {
  struct {
    SequenceNumber unack;
    SequenceNumber next;
    // WindowSize window; // replaced by |ring_buffer()->remaining()|
    SequenceNumber urgent_pointer;
    SequenceNumber last_update_seq;
    SequenceNumber last_update_ack;
    SequenceNumber initial;

    ALL_MODULO_COMPARISION
  } send{};

  struct {
    SequenceNumber next;
    // WindowSize window; // equal to free space size of |ring_buffer_|
    SequenceNumber urgent_pointer;
    SequenceNumber initial;

    // Other fields make sense only after we receive SYN from the peer.
    bool SYN_received;

    ALL_MODULO_COMPARISION
  } receive{};

  /* C++11 behavior: `Struct Name { ... } obj{}` without user-provided default
   * constructor should zero-initialize all fields of `obj`.
   *
   * See https://en.cppreference.com/w/cpp/language/value_initialization.
   */
};

#undef ALL_MODULO_COMPARISION
#undef MODULO_COMPARISIONS

#endif // TCPSTACK_TCB_H
