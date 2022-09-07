//
// Created by Chengke Wong on 2019/9/26.
//

#ifndef TCPSTACK_MAC_ADDRESS_H
#define TCPSTACK_MAC_ADDRESS_H

#include "../base/data_reader.h"
#include "../base/data_writer.h"
#include "./type.h"
#include <netinet/ether.h>

/**
 * @brief Wrapper of Linux MAC address `ether_addr`
 *
 */
class MacAddress {
public:
  MacAddress() = default;

  MacAddress(const char *text);
  explicit MacAddress(DataReader *reader);
  explicit MacAddress(const ether_addr &addr);

  static MacAddress fromDeviceName(const char *device_name);
  static MacAddress broadcastAddress() { return {"FF:FF:FF:FF:FF:FF"}; };

  void writeTo(DataWriter *writer);
  std::string toString();

  inline bool isSpecified() { return is_spec_; }
  bool isBroadcast();

  friend bool operator==(MacAddress lhs, MacAddress rhs);
  friend bool operator!=(MacAddress lhs, MacAddress rhs);

private:
  bool is_spec_ = false;
  ether_addr address_;
};

#endif // TCPSTACK_MAC_ADDRESS_H
