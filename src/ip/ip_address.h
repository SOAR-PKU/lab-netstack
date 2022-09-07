//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef SRC_IP_IP_ADDRESS_H
#define SRC_IP_IP_ADDRESS_H

#include "../base/data_reader.h"
#include "../base/data_writer.h"
#include "./type.h"
#include <netinet/in.h>

enum class IPAddressFamily {
  IP_V4,
  IP_V6,
  IP_UNSPEC,
};

/**
 * @brief Wrapper of Linux inet address
 *
 */
class IPAddress {
public:
  IPAddress();
  IPAddress(const IPAddress &other) = default;
  IPAddress(const char *text);
  explicit IPAddress(DataReader *reader);
  explicit IPAddress(const in_addr &ipv4_address);
  IPAddress &operator=(const IPAddress &other) = default;
  IPAddress &operator=(IPAddress &&other) = default;
  friend bool operator==(IPAddress lhs, IPAddress rhs);
  friend bool operator!=(IPAddress lhs, IPAddress rhs);
  friend IPAddress operator&(IPAddress lhs, IPAddress rhs);

  static IPAddress fromDeviceName(const char *device_name);
  static IPAddress fullMask() { return {"255.255.255.255"}; }
  static IPAddress inaddrAny() { return {"0.0.0.0"}; }

  bool writeTo(DataWriter *writer);

  std::string toString() const;

  // to use IP address as key of hash maps
  struct IpAddressHash {
    inline size_t operator()(const IPAddress &address) const {
      return address.address_.v4.s_addr;
    }
  };

  bool isInaddrAny() const;

private:
  union {
    in_addr v4;
    in6_addr v6;
    char bytes[kMaxIpAddressSize];
  } address_;
  IPAddressFamily family_;
};

#endif // SRC_IP_IP_ADDRESS_H
