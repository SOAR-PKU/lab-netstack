//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef SRC_TCP_TYPE_H
#define SRC_TCP_TYPE_H

#include "../base/time_base.h"
#include <cstddef>
#include <cstdint>

const size_t kTcpHeaderLength = 20;
const size_t kMaxSegmentSize = 1460; // MSS
const size_t kMaxTCPPacketLength = 1480;

// Maximum Segment Lifetime (MSL)
const TimeBase::Delta kTwoMSL = TimeBase::Delta::fromSeconds(10);

typedef uint16_t SocketPort;
typedef uint32_t SequenceNumber;

typedef uint16_t WindowSize;

#endif // SRC_TCP_TYPE_H
