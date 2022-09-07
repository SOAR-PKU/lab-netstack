//
// Created by Chengke Wong on 2019/9/26.
//

#include "mac_address.h"
#include "../base/data_reader.h"
#include "../posix/wrap_function.h"
#include <cstring>
#include <fstream>
#include <net/if.h>
#include <sys/ioctl.h>
#include <zconf.h>

void MacAddress::writeTo(DataWriter *writer) {
  writer->writeBytes(address_.ether_addr_octet, kMacAddressLength);
}

std::string MacAddress::toString() {
  char *rv = ether_ntoa(&address_);
  return std::string(rv);
}

/**
 * @brief Get device mac address using system provided real socket functions.
 *
 * @param device_name should be found by a `pcap_if_t`
 * @return MacAddress
 */
MacAddress MacAddress::fromDeviceName(const char *device_name) {
  /* Create an ifreq structure for passing data in and out of ioctl */
  struct ifreq ifr {};
  size_t if_name_len = strlen(device_name);
  if (if_name_len < sizeof(ifr.ifr_name)) {
    memcpy(ifr.ifr_name, device_name, if_name_len);
    ifr.ifr_name[if_name_len] = 0;
  } else {
    LOG(ERROR) << "The name is too long: " << device_name;
    return {};
  }

  /* Provide an open socket descriptor */
  int fd = __real_socket(AF_UNIX, SOCK_DGRAM, 0);
  if (fd == -1) {
    LOG(ERROR) << "socket failed " << strerror(errno) << ": " << device_name;
    return {};
  }

  /* Invoke ioctl */
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
    LOG(ERROR) << "ioctl failed " << strerror(errno) << ": " << device_name;
    __real_close(fd);
    return {};
  }
  __real_close(fd);

  /* Check the type of the returned hardware address */
  if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
    LOG(ERROR) << device_name << " is not Ethernet";
    return {};
  }

  struct ether_addr rv {};
  memcpy(&rv, ifr.ifr_hwaddr.sa_data, sizeof(struct ether_addr));

  return MacAddress(rv);
}

MacAddress::MacAddress(const ether_addr &addr) : is_spec_(true), address_() {
  memcpy(&address_, &addr, sizeof(ether_addr));
}

MacAddress::MacAddress(DataReader *reader) : is_spec_(true), address_() {
  reader->readBytes(&address_.ether_addr_octet, kMacAddressLength);
}

bool MacAddress::isBroadcast() {
  for (unsigned char i : address_.ether_addr_octet) {
    if (i != 0xff) {
      return false;
    }
  }
  return true;
}

bool operator==(MacAddress lhs, MacAddress rhs) {
  return memcmp(lhs.address_.ether_addr_octet, rhs.address_.ether_addr_octet,
                kMacAddressLength) == 0;
}

bool operator!=(MacAddress lhs, MacAddress rhs) { return !(lhs == rhs); }

MacAddress::MacAddress(const char *text) : is_spec_(true), address_() {
  ether_addr *rv = ether_aton(text);
  if (rv == nullptr) {
    is_spec_ = false;
    return;
  }
  memcpy(&address_, rv, sizeof(ether_addr));
}
