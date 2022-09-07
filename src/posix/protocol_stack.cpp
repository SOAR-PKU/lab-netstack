//
// Created by Chengke Wong on 2019/10/24.
//

#include "protocol_stack.h"
#include "../tcp/socket_address.h"
#include "wrap_function.h"
#include <fcntl.h>
#include <string.h>
#include <thread>
#include <zconf.h>

#define RETURN_ERRNO(err)                                                      \
  do {                                                                         \
    errno = err;                                                               \
    return -1;                                                                 \
  } while (false)

ProtocolStack::ProtocolStack()
    : epoll_server_(std::make_unique<EpollServer>()),
      device_manager_(std::make_unique<DeviceManager>(epoll_server_.get())),
      alarm_factory_(std::make_unique<EpollAlarmFactory>(epoll_server_.get())),
      ip_layer_(std::make_unique<IPLayer>(device_manager_.get(),
                                          alarm_factory_.get())),
      rand_generator_(19260817), // MAGIC NUMBER
      reset_dispatcher_(std::make_unique<ResetDispatcher>(
          ip_layer_.get(), alarm_factory_.get())),
      listen_dispatcher_(std::make_unique<ListenDispatcher>(
          reset_dispatcher_.get(), ip_layer_.get(), alarm_factory_.get(),
          &rand_generator_)),
      dispatcher_(
          std::make_unique<SegmentDispatcher>(listen_dispatcher_.get())),
      null_fd_(open("/dev/null", 0, 0)) {
  if (null_fd_ < 0) {
    LOG(FATAL) << "can not create null fd. " << strerror(errno);
  }
  device_manager_->setCallback(ip_layer_.get());
  ip_layer_->setCallback(this);
  listen_dispatcher_->setDispatcher(dispatcher_.get());
}

/**
 * @brief Create a ProtocolStack instance and spawn a thread at first call.
 * Return the instance otherwise.
 *
 * @return ProtocolStack&
 */
ProtocolStack &ProtocolStack::getInstance() {
  static ProtocolStack *instance;
  static std::mutex mtx;

  if (instance == nullptr) {
    std::lock_guard<std::mutex> guard(mtx);
    if (instance == nullptr) {
      ProtocolStack *rv = new ProtocolStack();
      std::thread(ProtocolStack::startEventLoop, rv->epoll_server_.get())
          .detach();
      instance = rv;
    }
  }
  return *instance;
}

/**
 * @brief EpollServer is initiated here and all events are handled through its
 * callback chains
 *
 * @param epoll_server
 */
void ProtocolStack::startEventLoop(EpollServer *epoll_server) {
  while (1) {
    epoll_server->runEventLoop(TimeBase::Delta::fromMilliseconds(1000));
  }
}

int ProtocolStack::_socket(int domain, int type, int protocol) {
  if ((domain != AF_INET) || (type != SOCK_STREAM) ||
      (protocol != 0 && protocol != IPPROTO_TCP)) {
    return __real_socket(domain, type, protocol);
  }

  int fd = dup(null_fd_);
  DCHECK(!ownFD(fd));
  SocketAddress socket_address(ip_layer_->getDefaultIP(),
                               // IANA: dynamic and private ports
                               rand_generator_.rand(49162, 65536));

  fd_set_[fd] = std::make_unique<SocketStruct>(socket_address);
  return fd;
}

bool ProtocolStack::ownFD(int fd) { return fd_set_.find(fd) != fd_set_.end(); }

SocketStruct *ProtocolStack::lookupStruct(int fd) {
  auto iter = fd_set_.find(fd);
  if (iter == fd_set_.end()) {
    return nullptr;
  }
  return iter->second.get();
}

void ProtocolStack::removeStruct(int fd) { fd_set_.erase(fd); }

int ProtocolStack::_bind(int socket, const struct sockaddr *address,
                         socklen_t address_len) {
  if (!ownFD(socket)) {
    return __real_bind(socket, address, address_len);
  }

  std::lock_guard<std::mutex> guard(mutex_);
  SocketAddress socket_address(address, address_len);

  SocketStruct *socket_struct = fd_set_[socket].get();
  socket_struct->bind_address = socket_address;
  return 0;
}

int ProtocolStack::_listen(int socket, int backlog) {
  if (!ownFD(socket)) {
    return __real_listen(socket, backlog);
  }
  if (backlog <= 0) {
    /**
     * see IEEE Std 1003.1-2017
     *
     * If listen() is called with a backlog argument value that is less than 0,
     * the function behaves as if it had been called with a backlog argument
     * value of 0.
     *
     * A backlog argument of 0 may allow the socket to accept connections, in
     * which case the length of the listen queue may be set to an
     * implementation-defined minimum value.
     */
    backlog = 1;
  }

  std::lock_guard<std::mutex> guard(mutex_);
  SocketStruct *st = lookupStruct(socket);
  if (st->type != SocketStruct::Type::UNSPECIFIED) {
    // The socket is already connected.
    RETURN_ERRNO(EINVAL);
  }

  st->type = SocketStruct::Type::PASSIVE;
  st->backlog = backlog;

  listen_dispatcher_->addListener(st->bind_address, st);

  return 0;
}

int ProtocolStack::_connect(int socket, const struct sockaddr *address,
                            socklen_t address_len) {
  if (!ownFD(socket)) {
    return __real_connect(socket, address, address_len);
  }

  std::lock_guard<std::mutex> guard(mutex_);
  SocketStruct *st = lookupStruct(socket);
  SocketAddress peer_address(address, address_len);
  if (st->type == SocketStruct::Type::PASSIVE) {
    // The socket is listening and cannot be connected.
    RETURN_ERRNO(EOPNOTSUPP);
  }
  if (st->type == SocketStruct::Type::ACTIVE) {
    // The specified socket is connection-mode and is already connected.
    RETURN_ERRNO(EISCONN);
  }
  DCHECK(st->type == SocketStruct::Type::UNSPECIFIED);
  st->type = SocketStruct::Type::ACTIVE;

  st->session = std::move(std::make_unique<SocketSession>(
      ip_layer_.get(), alarm_factory_.get(), st->bind_address, peer_address,
      &rand_generator_));
  dispatcher_->addSession(st->bind_address, peer_address, st->session.get());

  st->session->open();
  st->session->setCallback(st);
  return 0;
}

/**
 * @brief Accept an incoming session in the pending queue and return a file
 * desicriptor.
 *
 *
 * @param socket
 * @param address
 * @param address_len
 * @return int, a file descriptor
 */
int ProtocolStack::_accept(int socket, struct sockaddr *address,
                           socklen_t *address_len) {
  if (!ownFD(socket)) {
    return __real_accept(socket, address, address_len);
  }

  std::unique_lock<std::mutex> guard(mutex_);
  SocketStruct *st = lookupStruct(socket);
  if (st->type != SocketStruct::Type::PASSIVE) {
    // The socket is not accepting connections.
    RETURN_ERRNO(EINVAL);
  }

  while (true) {
    st->waitMessage(guard);
    // LAB: insert your code here.

  }

  return 0;
}

/**
 * @brief Read up to `nbyte` from socket
 * @see SocketSession::receive, ProtocolStack::_write
 *
 * @param fildes
 * @param buf
 * @param nbyte maximum bytes to be read
 * @return ssize_t bytes read
 */
ssize_t ProtocolStack::_read(int fildes, void *buf, size_t nbyte) {
  // LAB: insert your code here.

}

ssize_t ProtocolStack::_write(int fildes, const void *buf, size_t nbyte) {
  if (!ownFD(fildes)) {
    return __real_write(fildes, buf, nbyte);
  }

  std::unique_lock<std::mutex> guard(mutex_);
  SocketStruct *st = lookupStruct(fildes);

  if (st->type == SocketStruct::Type::UNSPECIFIED) {
    RETURN_ERRNO(ENOTCONN);
  }

  if (st->type != SocketStruct::Type::ACTIVE) {
    // The socket is not accepting connections.
    RETURN_ERRNO(EBADF);
  }

  size_t rv = st->session->send((char *)buf, nbyte);
  if (rv > 0) {
    return rv;
  }

  while (true) {
    st->waitMessage(guard);

    if (st->message_ & SocketSession::WRITABLE) {
      size_t rv = st->session->send((char *)buf, nbyte);
      if (rv > 0) {
        return rv;
      }
    }

    if (st->isErrorMessage()) {
      RETURN_ERRNO(ECONNABORTED);
    }
  }
}

int ProtocolStack::_close(int fildes) {
  if (!ownFD(fildes)) {
    return __real_close(fildes);
  }

  std::lock_guard<std::mutex> guard(mutex_);
  SocketStruct *st = lookupStruct(fildes);

  switch (st->type) {
  case SocketStruct::Type::UNSPECIFIED:
    break;
  case SocketStruct::Type::ACTIVE:
    st->session->close();
    garbage_.push_back(std::move(st->session));
    break;
  case SocketStruct::Type::PASSIVE:
    for (auto &e : st->pending) {
      e->close();
      garbage_.push_back(std::move(e));
    }
    break;
  default:
    LOG(FATAL) << "unknown socket type";
  }
  removeStruct(fildes);

  return 0;
}

int ProtocolStack::_getaddrinfo(const char *node, const char *service,
                                const struct addrinfo *hints,
                                struct addrinfo **res) {
  return __real_getaddrinfo(node, service, hints, res);
}

/**
 * @brief Call dispatchers to handle segments.
 * A dispatcher manages sessions according to their states:
 * 1. If its an established session, call segment dispatcher
 * 2. If not, then it is an half-opened session, call listen dispatcher
 * 3. Else, the TCB does not exist, call reset dispatcher
 * @param buffer
 * @param length
 * @param source
 * @param destination
 */
void ProtocolStack::onReceivePacket(char *buffer, size_t length,
                                    IPAddress source, IPAddress destination) {
  std::lock_guard<std::mutex> guard(mutex_);
  dispatcher_->onReceivePacket(buffer, length, source, destination);
}

ProtocolStack::~ProtocolStack() {
  while (!fd_set_.empty()) {
    auto iter = fd_set_.begin();
    _close(iter->first);
  }
}

void ProtocolStack::closeAllFDs() {
  while (!fd_set_.empty()) {
    _close(fd_set_.begin()->first);
  }
}

bool ProtocolStack::canClose() { return garbage_.empty(); }
