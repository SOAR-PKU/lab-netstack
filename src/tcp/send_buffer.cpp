//
// Created by Chengke Wong on 2019/10/22.
// Modified by Chengke on 2022/09/03.
//

#include "send_buffer.h"

SendBuffer::SendBuffer(IPLayer *ip_layer, AlarmFactory *alarm_factory,
                       SegmentFactory *segment_factory, ControlBlock *tcb)
    : ip_layer_(ip_layer),
      retransmission_alarm_(alarm_factory->createAlarm(
          std::make_unique<RetransmissionAlarmDelegate>(this))),
      alarm_factory_(alarm_factory), segment_factory_(segment_factory),
      buffer_(1024 * 256 /* 256 KB */), SYN_(CtlState::None),
      FIN_(CtlState::None), tcb_(tcb), bytes_in_flight_(0) {
  retransmission_alarm_->set(alarm_factory->now() +
                             TimeBase::Delta::fromSeconds(1));
}

void SendBuffer::sendSYN(SequenceNumber initial) {
  DCHECK(FIN_ == CtlState::None);

  tcb_->send.initial = initial;
  tcb_->send.next = initial + 1;
  tcb_->send.unack = initial;
  SYN_ = CtlState::Sent;
  transmit();
}

size_t SendBuffer::sendData(char *data, size_t length) {
  DCHECK(FIN_ == CtlState::None);
  if (buffer_.full()) {
    return 0;
  }

  size_t consumed = buffer_.write(data, length);
  tcb_->send.next += consumed;

  if (SYN_acked()) {
    transmit();
  }

  return consumed;
}

void SendBuffer::sendFIN() {
  FIN_ = CtlState::Sent;
  tcb_->send.next += 1;

  if (buffer_.empty()) {
    // All data has been acknowledged. We can send FIN at once.
    transmit();
  }
}

bool SendBuffer::sendSegment(Segment *segment) {
  // DLOG(INFO) << *segment;

  char buf[kMaxTCPPacketLength];
  DataWriter writer(buf, kMaxTCPPacketLength);
  segment->writeSegmentTo(&writer);

  if (!ip_layer_->sendPacket(source_, destination_, ServiceProtocol::TCP, buf,
                             writer.bytesWritten())) {
    DLOG(ERROR) << "sendSegment failed";
    return false;
  }
  return true;
}

/**
 * @brief Acknowledge segments in the sending queue up to sequence number |ack|
 * A segment is removed from the sending queue if it is acknowledged.
 *
 * @param ack
 */
void SendBuffer::acknowledge(SequenceNumber ack) {
  if (SYN_ == CtlState::Sent) {
    DCHECK(tcb_->send.greaterThan(ack, tcb_->send.initial));
    SYN_ = CtlState::Acked;
    tcb_->send.unack = tcb_->send.initial + 1;
  }

  DCHECK(tcb_->send.greaterOrEqual(ack, tcb_->send.unack));
  size_t consumed = ack - tcb_->send.unack;
  tcb_->send.unack += buffer_.consume(consumed);
  bytes_in_flight_ -= consumed;

  if (bytes_in_flight_ == 0) {
    // All our sent data have been acked.
    retransmission_alarm_->update(alarm_factory_->now() +
                                  TimeBase::Delta::fromSeconds(1));
  }

  if (FIN_ == CtlState::Sent) {
    if (tcb_->send.equal(ack, tcb_->send.next)) {
      DCHECK(buffer_.empty());
      FIN_ = CtlState::Acked;
      retransmission_alarm_->cancel();
      tcb_->send.unack += 1;
    }
  }

  if (consumed > 0) {
    // We can send more data
    transmit();
  }
}

void SendBuffer::transmit() {
  if (SYN_ == CtlState::None) {
    return;
  }
  if (SYN_ == CtlState::Sent) {
    return transmit_SYN();
  }

  char buf[kMaxTCPPacketLength];
  while (bytes_in_flight_ < kMaxBytesInFlight) {
    size_t length = buffer_.read_offset(bytes_in_flight_, buf, kMaxSegmentSize);
    SequenceNumber seq = tcb_->send.unack + bytes_in_flight_;

    bytes_in_flight_ += length;
    SegmentFlags flags = Segment::ACK;

    if (FIN_ == CtlState::Sent &&
        tcb_->send.equal(seq + length + 1, tcb_->send.next)) {
      // piggyback the FIN
      flags |= Segment::FIN;
    } else if (length == 0) {
      // the application doesn't have enough data to send
      return;
    }

    auto segment = segment_factory_->createSegment(seq, tcb_->receive.next,
                                                   flags, buf, length);
    if (!sendSegment(segment.get())) {
      return;
    }
    if (length == 0) {
      DCHECK(FIN_ == CtlState::Sent);
      return;
    }
  }
}

void SendBuffer::transmit_SYN() {
  if (tcb_->receive.SYN_received) {
    auto segment = segment_factory_->createSegment(
        tcb_->send.initial, tcb_->receive.next, Segment::SYN | Segment::ACK);
    bool ignore = sendSegment(segment.get());
  } else {
    auto segment =
        segment_factory_->createSegment(tcb_->send.initial, Segment::SYN);
    bool ignore = sendSegment(segment.get());
  }
}

void SendBuffer::retransmit() {
  bytes_in_flight_ = 0;
  transmit();
  retransmission_alarm_->set(alarm_factory_->now() +
                             TimeBase::Delta::fromSeconds(1));
}
