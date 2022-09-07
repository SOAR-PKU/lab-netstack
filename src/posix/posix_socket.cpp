//
// Created by Chengke Wong on 2019/9/30.
//

#include "posix_socket.h"
#include "protocol_stack.h"

int __wrap_socket(int domain, int type, int protocol) {
  return ProtocolStack::getInstance()._socket(domain, type, protocol);
}

int __wrap_bind(int socket, const struct sockaddr *address,
                socklen_t address_len) {
  return ProtocolStack::getInstance()._bind(socket, address, address_len);
}

int __wrap_listen(int socket, int backlog) {
  return ProtocolStack::getInstance()._listen(socket, backlog);
}

int __wrap_connect(int socket, const struct sockaddr *address,
                   socklen_t address_len) {
  return ProtocolStack::getInstance()._connect(socket, address, address_len);
}

int __wrap_accept(int socket, struct sockaddr *address,
                  socklen_t *address_len) {
  return ProtocolStack::getInstance()._accept(socket, address, address_len);
}

ssize_t __wrap_read(int fildes, void *buf, size_t nbyte) {
  return ProtocolStack::getInstance()._read(fildes, buf, nbyte);
}

ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte) {
  return ProtocolStack::getInstance()._write(fildes, buf, nbyte);
}

int __wrap_close(int fildes) {
  return ProtocolStack::getInstance()._close(fildes);
}

int __wrap_getaddrinfo(const char *node, const char *service,
                       const struct addrinfo *hints, struct addrinfo **res) {
  return ProtocolStack::getInstance()._getaddrinfo(node, service, hints, res);
}

void __destory_protocol_stack() { ProtocolStack::getInstance().closeAllFDs(); }

int __can_close_protocol_stack() {
  return ProtocolStack::getInstance().canClose();
}