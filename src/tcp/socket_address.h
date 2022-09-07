//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef SRC_TCP_SOCKET_ADDRESS_H
#define SRC_TCP_SOCKET_ADDRESS_H

#include "../ip/ip_address.h"
#include "./type.h"

/**
 * @brief A socket address consists of an IP address and a port number.
 *
 */
class SocketAddress {
public:
  SocketAddress() = default;
  SocketAddress(IPAddress address, SocketPort port);
  explicit SocketAddress(const struct sockaddr_storage &saddr);
  SocketAddress(const sockaddr *saddr, socklen_t len);
  SocketAddress(const SocketAddress &other) = default;
  SocketAddress &operator=(const SocketAddress &other) = default;
  SocketAddress &operator=(SocketAddress &&other) = default;

  friend bool operator==(SocketAddress lhs, SocketAddress rhs);

  std::string toString() const;

  inline SocketPort port() const { return port_; }

  inline IPAddress ipAddress() const { return host_; }

private:
  IPAddress host_;
  SocketPort port_ = 0; // host byte end
};

#endif // SRC_TCP_SOCKET_ADDRESS_H
