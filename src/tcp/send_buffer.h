//
// Created by Chengke Wong on 2019/10/22.
//

#ifndef SRC_TCP_SENDING_QUEUE_H
#define SRC_TCP_SENDING_QUEUE_H

#include "../base/ring_buffer.h"
#include "../base/util.h"
#include "../ip/ip_layer.h"
#include "control_block.h"
#include "segment.h"
#include "segment_factory.h"
#include <queue>

/**
 * @brief SendBuffer is responsable for FIFO segments sending and
 * retransmission. An ACK segment from peer could remove segments in the queue.
 * @see Member function |acknowledge|.
 *
 * It is our job to build the TCP header, and pass the packet down to IP so it
 * can do the same plus passing the packet off to the device.
 */
class SendBuffer {
public:
  DISALLOW_COPY_AND_ASSIGN(SendBuffer)

  SendBuffer(IPLayer *ip_layer, AlarmFactory *alarm_factory,
             SegmentFactory *segment_factory, ControlBlock *tcb);

  void sendSYN(SequenceNumber initial);

  size_t sendData(char *data, size_t length);

  void sendFIN();

  /**
   * @brief Send data in the `buffer_` to fullfill the sending window.
   * Since we do not implement flow control, the sending window size
   * is fixed to be `kMaxBytesInFlight`
   */
  void transmit();

  void acknowledge(SequenceNumber ack);

  // Immediately send the segment to the IP layer.
  bool sendSegment(Segment *seg);

  inline void setSource(const IPAddress &source) { source_ = source; }

  inline void setDestination(const IPAddress &destination) {
    destination_ = destination;
  }

  inline bool SYN_acked() const { return SYN_ == CtlState::Acked; }

  inline bool FIN_acked() const { return FIN_ == CtlState::Acked; }

private:
  static constexpr size_t kMaxBytesInFlight = kMaxSegmentSize * 10;

  void transmit_SYN();

  /**
   * @brief The retransmission behavior is simple: call `transmit` every 1s
   * i.e. RTO is fixed to 1s
   * so that all unacked segments are retransmitted.
   * @todo Implement adaptive RTO
   */
  void retransmit();

  RingBuffer buffer_;

  ControlBlock *tcb_;

  enum CtlState : u_int8_t {
    None,  // Unspecified
    Sent,  // Needs to send
    Acked, // Acked by peer
  };

  CtlState SYN_, FIN_;

  // bytes_in_flight_ <= min( rwnd, cwnd )
  size_t bytes_in_flight_;

  IPAddress source_;
  IPAddress destination_;

  AlarmFactory *alarm_factory_;
  SegmentFactory *segment_factory_;

  IPLayer *ip_layer_;

  DEFINE_ALARM_DELEGATE(RetransmissionAlarmDelegate, SendBuffer, retransmit);
  std::unique_ptr<Alarm> retransmission_alarm_;
};

#endif // SRC_TCP_SENDING_QUEUE_H
