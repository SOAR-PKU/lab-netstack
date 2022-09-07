//
// Created by Chengke Wong on 2019/9/27.
//

#include "socket_address.h"
#include <cstring>
#include <glog/logging.h>
#include <limits>

SocketAddress::SocketAddress(IPAddress address, SocketPort port)
    : host_(address), port_(port) {}

SocketAddress::SocketAddress(const struct sockaddr_storage &saddr) {
  switch (saddr.ss_family) {
  case AF_INET: {
    const auto *v4 = reinterpret_cast<const sockaddr_in *>(&saddr);
    host_ = IPAddress(v4->sin_addr);
    port_ = ntohs(v4->sin_port);
    break;
  }
  case AF_INET6:
  default:
    LOG(ERROR) << "Unknown address family passed: " << saddr.ss_family;
    break;
  }
}

SocketAddress::SocketAddress(const sockaddr *saddr, socklen_t len) {
  struct sockaddr_storage storage {};
  static_assert(std::numeric_limits<socklen_t>::max() >= sizeof(storage),
                "Cannot cast sizeof(storage) to socklen_t as it does not fit");
  if (len < static_cast<socklen_t>(sizeof(sockaddr)) ||
      (saddr->sa_family == AF_INET &&
       len < static_cast<socklen_t>(sizeof(sockaddr_in))) ||
      (saddr->sa_family == AF_INET6 &&
       len < static_cast<socklen_t>(sizeof(sockaddr_in6))) ||
      len > static_cast<socklen_t>(sizeof(storage))) {
    LOG(ERROR) << "Socket address of invalid length provided";
    return;
  }
  memcpy(&storage, saddr, len);
  *this = SocketAddress(storage);
}

bool operator==(SocketAddress lhs, SocketAddress rhs) {
  return lhs.port() == rhs.port() && lhs.ipAddress() == rhs.ipAddress();
}

std::string SocketAddress::toString() const {
  return ipAddress().toString() + ":" + std::to_string(port());
}
