//
// Created by Chengke Wong on 2019/9/30.
//

#include "checksum.h"
#include <cstddef>
#include <netinet/in.h>

Checksum::Checksum(const char *begin, const char *end) {
  size_t length = end - begin;
  uint32_t sum = 0;
  if (length & 1u) {
    end -= 1;
    sum = (uint16_t)(*((uint8_t *)end)) << 8u;
  }

  auto *data = (uint16_t *)begin;
  auto *data_end = (uint16_t *)end;
  while (data < data_end) {
    sum += ntohs(*data);
    sum = (sum & 0xffffu) + ((sum >> 16u));
    data += 1;
  }

  sum_ = sum;
}

void Checksum::addHostUInt16(uint16_t value) {
  uint32_t sum = sum_;
  sum += value;
  sum = (sum & 0xffffu) + ((sum >> 16u));
  sum_ = sum;
}

void Checksum::addIPAddress(IPAddress ip_address) {
  uint32_t buffer;
  DataWriter writer((char *)&buffer, 8);
  DCHECK(ip_address.writeTo(&writer));
  addNetworkUint16(buffer >> 16u);
  addNetworkUint16(buffer);
}

void Checksum::writeTo(char *buffer) {
  uint16_t cs = ~sum_;
  *buffer = (char)(cs >> 8u);
  *(buffer + 1) = (char)cs;
}

void Checksum::addNetworkUint16(uint16_t value) {
  value = ntohs(value);
  addHostUInt16(value);
}
