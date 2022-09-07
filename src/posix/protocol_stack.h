//
// Created by Chengke Wong on 2019/10/24.
//

#ifndef SRC_POSIX_PROTOCOL_STACK_H
#define SRC_POSIX_PROTOCOL_STACK_H

#include "../base/epoll_alarm_factory.h"
#include "../base/epoll_server.h"
#include "../base/util.h"
#include "../ether/device_manager.h"
#include "../ip/ip_layer.h"
#include "../tcp/socket_session.h"
#include "listen_dispatcher.h"
#include "reset_dispatcher.h"
#include "segment_dispatcher.h"
#include "socket_struct.h"
#include <condition_variable>
#include <list>
#include <mutex>

/**
 * @brief Singleton pattern.
 * A ProtocolStack owns all layers and handles user POSIX calls.
 * It is also an IPacketCallback and implements |onReceivePacket|
 * @see IPacketCallback::onReceivePacket
 */
class ProtocolStack : public IPacketCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(ProtocolStack)

  static ProtocolStack &getInstance();

  int _socket(int domain, int type, int protocol);

  int _bind(int socket, const struct sockaddr *address, socklen_t address_len);

  int _listen(int socket, int backlog);

  int _connect(int socket, const struct sockaddr *address,
               socklen_t address_len);

  int _accept(int socket, struct sockaddr *address, socklen_t *address_len);

  ssize_t _read(int fildes, void *buf, size_t nbyte);

  ssize_t _write(int fildes, const void *buf, size_t nbyte);

  int _close(int fildes);

  int _getaddrinfo(const char *node, const char *service,
                   const struct addrinfo *hints, struct addrinfo **res);

  void onReceivePacket(char *buffer, size_t length, IPAddress source,
                       IPAddress destination) override;

  void closeAllFDs();

  bool canClose();

private:
  ProtocolStack();
  ~ProtocolStack() override;

  static void startEventLoop(EpollServer *epoll_server);

  std::mutex mutex_;

  std::unique_ptr<EpollServer> epoll_server_;
  std::unique_ptr<DeviceManager> device_manager_;
  std::unique_ptr<AlarmFactory> alarm_factory_;
  std::unique_ptr<IPLayer> ip_layer_;
  RandGenerator rand_generator_;
  int null_fd_;
  std::unordered_map<int, std::unique_ptr<SocketStruct>> fd_set_;
  std::list<std::unique_ptr<SocketSession>> garbage_;

  /* note that dispatchers should be constructed after
   * other components because of the dependency. Thus,
   * we should place the dispatchers after other class
   * members!
   *
   * on receive: segment dispatcher --> listen dispatcher --> reset dispatcher
   */
  std::unique_ptr<ResetDispatcher> reset_dispatcher_;
  std::unique_ptr<ListenDispatcher> listen_dispatcher_;
  std::unique_ptr<SegmentDispatcher> dispatcher_;

  bool ownFD(int fd);

  // if not found return nullptr
  SocketStruct *lookupStruct(int fd);

  void removeStruct(int fd);
};

#endif // SRC_POSIX_PROTOCOL_STACK_H
