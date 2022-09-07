//
// Created by Chengke Wong on 2019/9/28.
//

#ifndef SRC_TCP_SOCKET_SESSION_H
#define SRC_TCP_SOCKET_SESSION_H

#include "../base/ring_buffer.h"
#include "control_block.h"
#include "segment.h"
#include "segment_factory.h"
#include "send_buffer.h"
#include "socket_address.h"
#include "type.h"

enum class ConnectionState {
  LISTEN,
  SYN_SENT,
  SYN_RECEIVED,
  ESTABLISHED,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSE_WAIT,
  CLOSING,
  LAST_ACK,
  TIME_WAIT,
  CLOSED,
};

/**
 * @brief A SocketSession is an abstraction of a TCP connection.
 * It handles the TCP state machine and propagate messages to upper socket
 * interface.
 *
 */
class SocketSession {
public:
  typedef unsigned int CallbackMessage;

  enum {
    READABLE = 0x001,
    WRITABLE = 0x002,
    CLOSED = 0x004,    // CLOSE can be acknowledged
    CLOSING = 0x008,   // receive FIN
    RESET = 0x010,     // connection reset
    REFUSED = 0x020,   // connection refused
    NOTEXIST = 0x040,  // connection does not exist
    NOSERVICE = 0x080, // connection closing and do not service request
  };

  /**
   * @brief Transport layer callback for handing packets to upper user
   * interface. POSIX socket interface should derive and implement this callback
   * interface.
   *
   */
  class Callback {
  public:
    Callback() = default;

    ~Callback() = default;
    DISALLOW_COPY_AND_ASSIGN(Callback)

    virtual void onMessage(CallbackMessage message) = 0;
  };

  DISALLOW_COPY_AND_ASSIGN(SocketSession)

  SocketSession(IPLayer *ip_layer, AlarmFactory *alarm_factory,
                SocketAddress local, SocketAddress remote, RandGenerator *rand);

  void onSegmentArrival(std::unique_ptr<Segment> segment);

  size_t send(char *data, size_t length);

  size_t receive(char *data, size_t length);

  void open();

  void open(std::unique_ptr<Segment> segment);

  void close();

  void setCallback(Callback *callback);

  bool isClosed();

  inline ConnectionState state() const { return state_; }

  SocketAddress getLocalAddress() const;
  SocketAddress getRemoteAddress() const;

private:
  void signalUser(CallbackMessage message);

  template <class... T> void sendSegmentNow(T &&...args) {
    std::unique_ptr<Segment> seg =
        factory_.createSegment(std::forward<T>(args)...);
    send_buffer_->sendSegment(seg.get());
  }

  ControlBlock tcb_;

  ConnectionState state_ = ConnectionState::CLOSED;

  // not owned
  IPLayer *output_;

  SegmentFactory factory_;
  // construct |send_buffer_| after |factory_| due to the dependency.
  std::unique_ptr<SendBuffer> send_buffer_;

  // not owned
  RandGenerator *rand_generator_;

  void onSegmentArrival_LISTEN(std::unique_ptr<Segment> segment);

  void onSegmentArrival_CLOSED(std::unique_ptr<Segment> segment);

  void onSegmentArrival_SYN_SENT(std::unique_ptr<Segment> segment);

  void onSegmentArrival_OTHERWISE(std::unique_ptr<Segment> segment);

  // user interface, not owned
  Callback *callback_;

  std::unique_ptr<RingBuffer> receive_buffer_;

  AlarmFactory *alarm_factory_;

  void enterCLOSED();

  DEFINE_ALARM_DELEGATE(TimeWaitDelegate, SocketSession, enterCLOSED);
  /**
   * Alarm for TIME_WAIT state.
   * Transmit the state to CLOSED after two MSL
   */
  std::unique_ptr<Alarm> time_wait_alarm_;
};

#endif // SRC_TCP_SOCKET_SESSION_H
