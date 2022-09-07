//
// Created by Chengke Wong on 2019/11/4.
//

#include "reset_dispatcher.h"
#include "../tcp/segment.h"
#include "../tcp/segment_factory.h"
#include "../tcp/socket_address.h"

void ResetDispatcher::onReceivePacket(char *buffer, size_t length,
                                      IPAddress source, IPAddress destination) {
  // if this method is call, it means that the TCB for such a packet does not
  // exist

  std::unique_ptr<Segment> segment =
      Segment::parse(source, destination, buffer, length);
  SocketAddress &to = segment->destination_;
  SocketAddress &from = segment->source_;

  SegmentFactory factory(to, from);

  std::unique_ptr<Segment> response;
  if (segment->isACK()) {
    response = factory.createSegment(segment->acknowledgment_, Segment::RST);
  } else {
    response =
        factory.createSegment(0, segment->sequence_ + segment->data_length_,
                              Segment::RST | Segment::ACK);
  }

  DLOG(INFO) << segment->toString();

  DataWriter writer(buffer_.get(), kMaxTCPPacketLength);
  segment->writeSegmentTo(&writer);

  if (!ip_layer_->sendPacket(to.ipAddress(), from.ipAddress(),
                             ServiceProtocol::TCP, buffer_.get(),
                             writer.bytesWritten())) {
    LOG(ERROR) << "sendSegment failed";
  }
}

ResetDispatcher::ResetDispatcher(IPLayer *ip_layer, AlarmFactory *alarm_factory)
    : buffer_(new char[kMaxTCPPacketLength]), ip_layer_(ip_layer),
      alarm_factory_(alarm_factory) {}
