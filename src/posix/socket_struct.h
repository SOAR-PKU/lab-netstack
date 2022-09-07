//
// Created by Chengke Wong on 2019/11/9.
//

#ifndef SRC_POSIX_SOCKET_STRUCT_H
#define SRC_POSIX_SOCKET_STRUCT_H

#include "../tcp/socket_session.h"
#include <condition_variable>
#include <mutex>

/**
 * @brief Abstraction of a POSIX socket.
 * An active socket can own only one SocketSession.
 * A passive socket can own a queue of SocketSessions.
 *
 */
class SocketStruct : public SocketSession::Callback {
public:
  enum class Type {
    PASSIVE,
    ACTIVE,
    UNSPECIFIED,
  };

  explicit SocketStruct(const SocketAddress &address);

  // active socket
  std::unique_ptr<SocketSession> session;
  // passive socket
  uint32_t backlog;
  std::vector<std::unique_ptr<SocketSession>> pending;

  // socket address
  SocketAddress bind_address;
  Type type;

  bool closing = false;

  // use mutex and conditional variable to ensure exclusive access
  // and save CPU cycles
  bool isErrorMessage() const;
  void onMessage(SocketSession::CallbackMessage message) override;
  void waitMessage(std::unique_lock<std::mutex> &mut);

  std::condition_variable cond_;

  SocketSession::CallbackMessage message_;
};

#endif // SRC_POSIX_SOCKET_STRUCT_H
