//
// Created by Chengke on 2019/10/28.
//

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

int __real_socket(int domain, int type, int protocol) { return -2; }

int __real_bind(int socket, const struct sockaddr *address,
                socklen_t address_len) {
  return -2;
}

int __real_listen(int socket, int backlog) { return -2; }

int __real_connect(int socket, const struct sockaddr *address,
                   socklen_t address_len) {
  return -2;
}

int __real_accept(int socket, struct sockaddr *address,
                  socklen_t *address_len) {
  return -2;
}

ssize_t __real_read(int fildes, void *buf, size_t nbyte) { return -2; }

ssize_t __real_write(int fildes, const void *buf, size_t nbyte) { return -2; }

ssize_t __real_close(int fildes) { return -2; }

int __real_getaddrinfo(const char *node, const char *service,
                       const struct addrinfo *hints, struct addrinfo **res) {
  return -2;
}
