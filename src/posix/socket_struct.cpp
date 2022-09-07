//
// Created by Chengke Wong on 2019/11/9.
//

#include "socket_struct.h"

SocketStruct::SocketStruct(const SocketAddress &address)
    : backlog(0), bind_address(address), type(Type::UNSPECIFIED) {}

void SocketStruct::onMessage(SocketSession::CallbackMessage message) {
  message_ |= message;
  if (message == SocketSession::CLOSING) {
    closing = true;
  }
  cond_.notify_one();
}

void SocketStruct::waitMessage(std::unique_lock<std::mutex> &mut) {
  message_ = 0;
  cond_.wait(mut);
}

bool SocketStruct::isErrorMessage() const {
  return message_ & ~static_cast<SocketSession::CallbackMessage>(
                        SocketSession::READABLE | SocketSession::WRITABLE);
}
