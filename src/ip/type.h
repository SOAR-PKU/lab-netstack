//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef SRC_IP_TYPE_H
#define SRC_IP_TYPE_H

const size_t kIPv4AddressSize = 32 / 8;
const size_t kIPv6AddressSize = 128 / 8;
const size_t kMaxIpAddressSize = kIPv6AddressSize;
const size_t kMaxIpPacketLength = 1500;
const size_t kPacketHeaderLength = 20;
const uint8_t kDefaultTTL = 64;
const size_t kTTLHeaderOffset = 8;

#endif // SRC_IP_TYPE_H
