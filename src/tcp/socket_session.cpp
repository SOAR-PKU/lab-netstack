//
// Created by Chengke Wong on 2019/9/28.
// Modified by Chengke on 2022/09/03.
//

#include "socket_session.h"

SocketSession::SocketSession(IPLayer *ip_layer, AlarmFactory *alarm_factory,
                             SocketAddress local, SocketAddress remote,
                             RandGenerator *rand)
    : factory_(local, remote), send_buffer_(std::make_unique<SendBuffer>(
                                   ip_layer, alarm_factory, &factory_, &tcb_)),
      output_(ip_layer), rand_generator_(rand), callback_(nullptr),
      receive_buffer_(std::make_unique<RingBuffer>(kMaxSegmentSize * 10)),
      time_wait_alarm_(
          alarm_factory->createAlarm(std::make_unique<TimeWaitDelegate>(this))),
      alarm_factory_(alarm_factory) {}

void SocketSession::setCallback(SocketSession::Callback *callback) {
  callback_ = callback;
}

/**
 * @brief Tell user interface something has happened in transport layer.
 *
 * @param message
 */
void SocketSession::signalUser(CallbackMessage message) {
  if (callback_ != nullptr) {
    callback_->onMessage(message);
  }
}

/**
 * @brief Handle segment arrival event following the TCP state machine rule.
 * Please read RFC 793 section 3.9 for comprehensive and detailed guidance.
 * @see https://www.rfc-editor.org/rfc/rfc793#section-3.9
 * @param segment
 */
void SocketSession::onSegmentArrival(std::unique_ptr<Segment> segment) {
  switch (state_) {
    // LAB: insert your code here.

  }
}

void SocketSession::onSegmentArrival_CLOSED(std::unique_ptr<Segment> segment) {
  // LAB: insert your code here.

}

void SocketSession::onSegmentArrival_LISTEN(std::unique_ptr<Segment> segment) {
  if (segment->isRST()) {
    return;
  }

  if (segment->isACK()) {
    sendSegmentNow(segment->acknowledgment_, Segment::RST);
    return;
  }

  if (segment->isSYN()) {
    /* We do not implement the security check. See section 3.6 of RFC 793.
     *
     * If the SYN bit is set, check the security. If the security/compartment
     * on the incoming segment does not exactly match the security/compartment
     * in the TCB then send a reset and return.
     *
     *   <SEQ=SEG.ACK><CTL=RST>
     *
     * If the SEG.PRC is greater than the TCB.PRC then if allowed by the user
     * and the system set TCB.PRC<-SEG.PRC, if not allowed send a reset and
     * return.
     *
     *   <SEQ=SEG.ACK><CTL=RST>
     *
     * If the SEG.PRC is less than the TCB.PRC then continue.
     */
    tcb_.receive.SYN_received = true;
    tcb_.receive.next = segment->sequence_ + 1;
    tcb_.receive.initial = segment->sequence_;

    /* RECOMMENDED: The generator is bound to a (possibly fictitious) 32 bit
     * clock whose low order bit is incremented roughly every 4 microseconds.
     * Thus, the ISN cycles approximately every 4.55 hours.
     */
    SequenceNumber initial =
        rand_generator_->rand(std::numeric_limits<SequenceNumber>::max());
    send_buffer_->sendSYN(initial);
    state_ = ConnectionState::SYN_RECEIVED;
    return;
  }
}

void SocketSession::onSegmentArrival_SYN_SENT(
    std::unique_ptr<Segment> segment) {
  // LAB: insert your code here.

}

void SocketSession::onSegmentArrival_OTHERWISE(
    std::unique_ptr<Segment> segment) {
  /*
   * Segments are processed in sequence. Initial tests on arrival are used to
   * discard old duplicates, but further processing is done in SEG.SEQ order.
   * If a segment's contents straddle the boundary between old and new, only
   * the new parts should be processed.
   */
  // LAB: insert your code here.

}

/**
 * @brief Handle send call from user.
 * @see RFC 793 section 3.9 SEND CALL
 * @param data
 * @param length
 * @return bytes sent.
 */
size_t SocketSession::send(char *data, size_t length) {
  // LAB: insert your code here.

}

/**
 * @brief Handle receive call from user.
 * @see RFC 793 section 3.9 RECEIVE CALL.
 * @param data
 * @param length
 * @return bytes read.
 */
size_t SocketSession::receive(char *data, size_t length) {
  // LAB: insert your code here.

}

/**
 * @brief Handle open call from user. This open is called by an active socket,
 * it tries to connect to the remote peer.
 * @see RFC 793 section 3.9 OPEN CALL
 */
void SocketSession::open() {
  if (state_ != ConnectionState::CLOSED) {
    LOG(FATAL) << "already open";
    return;
  }

  SequenceNumber initial =
      rand_generator_->rand(std::numeric_limits<SequenceNumber>::max());
  send_buffer_->sendSYN(initial);
  state_ = ConnectionState::SYN_SENT;
}

/**
 * @brief This open is called by a passive socket on receiving a SYN segment.
 * It turns its state to LISTEN and handles the SYN event.
 *
 * @param seg
 */
void SocketSession::open(std::unique_ptr<Segment> seg) {
  if (state_ != ConnectionState::CLOSED) {
    LOG(FATAL) << "already open";
    return;
  }

  state_ = ConnectionState::LISTEN;
  onSegmentArrival(std::move(seg));
}

bool SocketSession::isClosed() { return state_ == ConnectionState::CLOSED; }

/**
 * @brief Handles close call from user.
 * @see RFC 793 section 3.9 CLOSE CALL
 */
void SocketSession::close() {
  switch (state_) {
  case ConnectionState::CLOSED:
  case ConnectionState::LISTEN:
  case ConnectionState::SYN_SENT:
    state_ = ConnectionState::CLOSED;
    return;

  case ConnectionState::SYN_RECEIVED:
  case ConnectionState::ESTABLISHED:
    send_buffer_->sendFIN();
    state_ = ConnectionState::FIN_WAIT_1;
    break;

  case ConnectionState::FIN_WAIT_1:
  case ConnectionState::FIN_WAIT_2:
  case ConnectionState::CLOSING:
  case ConnectionState::LAST_ACK:
  case ConnectionState::TIME_WAIT:
    LOG(FATAL) << "error: connection closing";
    break;

  case ConnectionState::CLOSE_WAIT:
    send_buffer_->sendFIN();
    state_ = ConnectionState::LAST_ACK;
    break;

  default:
    LOG(FATAL) << "undefined behavior";
    return;
  }
}

SocketAddress SocketSession::getLocalAddress() const {
  return factory_.getLocalAddress();
}

SocketAddress SocketSession::getRemoteAddress() const {
  return factory_.getRemoteAddress();
}

/**
 * @brief
 *
 */
void SocketSession::enterCLOSED() { state_ = ConnectionState::CLOSED; }
