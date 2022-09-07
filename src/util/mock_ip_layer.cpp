//
// Created by Chengke Wong on 2019/11/5.
//

#include "mock_ip_layer.h"

bool MockIPLayer::sendPacket(IPAddress source, IPAddress destination,
                             ServiceProtocol proto, void *buf, size_t length) {

  if (queue_.size() >= kMaxQueueLength)
    return false;

  std::unique_ptr<Segment> segment =
      Segment::parse(source, destination, static_cast<char *>(buf), length);
  segment->copyData();

  DLOG(INFO) << "-> " << *segment;
  queue_.push_back(std::move(segment));

  return true;
}

MockIPLayer::MockIPLayer() : IPLayer() {}
