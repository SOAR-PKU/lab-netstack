//
// Created by Chengke on 2019/10/30.
//

#include "listen_dispatcher.h"

ListenDispatcher::ListenDispatcher(IPacketCallback *next, IPLayer *ip_layer,
                                   AlarmFactory *alarm_factory,
                                   RandGenerator *rand)
    : next_(next), ip_layer_(ip_layer), alarm_factory_(alarm_factory),
      rand_(rand), dispatcher_(nullptr) {}

void ListenDispatcher::onReceivePacket(char *buffer, size_t length,
                                       IPAddress source,
                                       IPAddress destination) {
  std::unique_ptr<Segment> segment =
      Segment::parse(source, destination, buffer, length);
  SocketAddress to = segment->destination_;
  SocketAddress from = segment->source_;

  SocketStruct *st;
  if ((st = lookupSession(to)) == nullptr) {
    next_->onReceivePacket(buffer, length, source, destination);
    return;
  }

  std::vector<std::unique_ptr<SocketSession>> *pending = &st->pending;
  if (pending->size() >= st->backlog) {
    LOG(INFO) << "too many incoming connections";
    return;
  }

  pending->push_back(std::move(std::make_unique<SocketSession>(
      ip_layer_, alarm_factory_, to, from, rand_)));

  pending->back()->open(std::move(segment));

  if (dispatcher_ == nullptr) {
    LOG(FATAL) << "don't forget to setup the |dispatcher_|";
    return;
  }
  dispatcher_->addSession(to, from, pending->back().get());
  st->cond_.notify_one();
}

void ListenDispatcher::addListener(SocketAddress target,
                                   SocketStruct *socket_st) {
  vector_.emplace_back(target, socket_st);
}

void ListenDispatcher::removeSession(SocketStruct *socket_st) {
  for (auto iter = vector_.begin(); iter != vector_.end(); iter++) {
    if (std::get<1>(*iter) == socket_st) {
      vector_.erase(iter);
      return;
    }
  }
  LOG(ERROR) << "can not find such a listening socket";
}

SocketStruct *ListenDispatcher::lookupSession(const SocketAddress &to) const {
  for (const auto &elem : vector_) {
    SocketAddress listener = std::get<0>(elem);
    bool port_match = to.port() == listener.port();
    bool ip_match = listener.ipAddress().isInaddrAny() ||
                    to.ipAddress() == listener.ipAddress();

    if (port_match && ip_match) {
      return std::get<1>(elem);
    }
  }
  return nullptr;
}

void ListenDispatcher::setDispatcher(SegmentDispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}
