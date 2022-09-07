//
// Created by Chengke Wong on 2019/10/22.
//

#include "segment_dispatcher.h"
#include "../tcp/segment.h"

SegmentDispatcher::SegmentDispatcher(IPacketCallback *next) : next_(next) {
  DCHECK(next != nullptr);
}

void SegmentDispatcher::onReceivePacket(char *buffer, size_t length,
                                        IPAddress source,
                                        IPAddress destination) {
  std::unique_ptr<Segment> segment =
      Segment::parse(source, destination, buffer, length);
  SocketAddress &to = segment->destination_;
  SocketAddress &from = segment->source_;

  DLOG(INFO) << "<- " << segment->toString();
  SocketSession *session = lookupSession(to, from);
  if (session == nullptr) {
    next_->onReceivePacket(buffer, length, source, destination);
    return;
  }

  // TODO: check the checksum

  session->onSegmentArrival(std::move(segment));
}

SocketSession *
SegmentDispatcher::lookupSession(const SocketAddress &to,
                                 const SocketAddress &from) const {
  for (const auto &elem : established_) {
    if (std::get<0>(elem) == to && std::get<1>(elem) == from) {
      return std::get<2>(elem);
    }
  }
  return nullptr;
}

void SegmentDispatcher::addSession(SocketAddress local, SocketAddress peer,
                                   SocketSession *session) {
  established_.emplace_back(local, peer, session);
}

void SegmentDispatcher::removeSession(SocketSession *session) {
  for (auto iter = established_.begin(); iter != established_.end(); iter++) {
    if (std::get<2>(*iter) == session) {
      established_.erase(iter);
      return;
    }
  }
  LOG(ERROR) << "can not find such a session";
}
