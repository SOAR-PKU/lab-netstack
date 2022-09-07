//
// Created by Chengke Wang on 2019/10/26.
//

#include "segment_factory.h"

SegmentFactory::SegmentFactory(SocketAddress local, SocketAddress remote)
    : local_address_(local), remote_address_(remote) {}

std::unique_ptr<Segment> SegmentFactory::createSegment(SequenceNumber seq,
                                                       SequenceNumber ack,
                                                       SegmentFlags ctl,
                                                       char *data,
                                                       size_t data_length) {
  auto segment = std::make_unique<Segment>();
  segment->source_ = local_address_;
  segment->destination_ = remote_address_;
  segment->sequence_ = seq;
  segment->acknowledgment_ = ack;
  segment->flags_ = ctl;

  // We don't implement the flow control so far.
  segment->window_ = 65535;

  segment->data_ = data;
  segment->own_data_ = false;
  segment->data_length_ = data_length;
  return segment;
}
