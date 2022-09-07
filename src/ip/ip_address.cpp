//
// Created by Chengke Wong on 2019/9/27.
//

#include "ip_address.h"

#include <algorithm>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../posix/wrap_function.h"

IPAddress::IPAddress(const in_addr &ipv4_address)
    : address_(), family_(IPAddressFamily::IP_V4) {
  address_.v4 = ipv4_address;
}

IPAddress::IPAddress() : address_(), family_(IPAddressFamily::IP_UNSPEC) {}

bool operator==(IPAddress lhs, IPAddress rhs) {
  if (lhs.family_ != rhs.family_) {
    return false;
  }

  switch (lhs.family_) {
  case IPAddressFamily::IP_V4:
    return std::equal(lhs.address_.bytes, lhs.address_.bytes + kIPv4AddressSize,
                      rhs.address_.bytes);
  case IPAddressFamily::IP_V6:
    return std::equal(lhs.address_.bytes, lhs.address_.bytes + kIPv6AddressSize,
                      rhs.address_.bytes);
  case IPAddressFamily::IP_UNSPEC:
    return true;
  }

  return false;
}

bool operator!=(IPAddress lhs, IPAddress rhs) { return !(lhs == rhs); }

IPAddress::IPAddress(DataReader *reader)
    : family_(IPAddressFamily::IP_V4), address_() {
  // TODO: support ipv6
  reader->readBytes(&address_.v4.s_addr, kIPv4AddressSize);
}

std::string IPAddress::toString() const {
  char str[INET_ADDRSTRLEN];
  if (family_ != IPAddressFamily::IP_V4 ||
      inet_ntop(AF_INET, &address_.v4, str, INET_ADDRSTRLEN) == nullptr) {
    return "unknown ipv4 address";
  }
  return str;
}

bool IPAddress::writeTo(DataWriter *writer) {
  if (family_ != IPAddressFamily::IP_V4) {
    LOG(FATAL) << "unsupported IP protocol";
    return false;
  }
  return writer->writeBytes(&address_.v4.s_addr, kIPv4AddressSize);
}

IPAddress IPAddress::fromDeviceName(const char *device_name) {
  int fd;
  struct ifreq ifr {};

  fd = __real_socket(AF_INET, SOCK_DGRAM, 0);

  if (fd < 0) {
    LOG(FATAL) << strerror(errno);
    return IPAddress();
  }

  /* get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, device_name, IFNAMSIZ - 1);
  int rv;
  rv = ioctl(fd, SIOCGIFADDR, &ifr);
  if (rv < 0) {
    LOG(FATAL) << strerror(errno);
    __real_close(fd);
    return IPAddress();
  }

  rv = __real_close(fd);
  if (rv < 0) {
    LOG(FATAL) << strerror(errno);
    return IPAddress();
  }

  in_addr addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
  return IPAddress(addr);
}

IPAddress::IPAddress(const char *text) : IPAddress() {
  int rv = inet_pton(AF_INET, text, &address_.v4);
  if (rv < 0) {
    LOG(ERROR) << text << " can't converted into ipv4 address";
    return;
  }

  family_ = IPAddressFamily::IP_V4;
}

IPAddress operator&(IPAddress lhs, IPAddress rhs) {
  if (lhs.family_ != rhs.family_) {
    return IPAddress();
  }
  switch (lhs.family_) {
  case IPAddressFamily::IP_V4:
    return IPAddress(in_addr{lhs.address_.v4.s_addr & rhs.address_.v4.s_addr});
  case IPAddressFamily::IP_V6:
  case IPAddressFamily::IP_UNSPEC:
    return IPAddress();
  }

  return IPAddress();
}

bool IPAddress::isInaddrAny() const {
  if (family_ != IPAddressFamily::IP_V4) {
    return false;
  }
  return address_.v4.s_addr == INADDR_ANY;
}
