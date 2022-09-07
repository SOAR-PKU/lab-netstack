//
// Created by Chengke Wong on 2019/9/30.
//

#ifndef SRC_BASE_CHECKSUM_H
#define SRC_BASE_CHECKSUM_H

#include "../ip/ip_address.h"
#include <cstdint>

class Checksum {
public:
  Checksum() = default;

  Checksum(const char *begin, const char *end);

  inline uint16_t getChecksum() { return sum_; }

  void addHostUInt16(uint16_t value);

  void addNetworkUint16(uint16_t value);

  void addIPAddress(IPAddress ip_address);

  void writeTo(char *buffer);

private:
  // in host endianness
  uint16_t sum_;
};

#endif // SRC_BASE_CHECKSUM_H
