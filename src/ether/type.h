//
// Created by Chengke Wong on 2019/9/26.
//

#ifndef TCPSTACK_TYPE_H
#define TCPSTACK_TYPE_H

const size_t kEtherDataLengthMin = 46;
const size_t kEtherDataLengthMax = 1500;
const size_t kMacAddressLength = 6;
const size_t kEtherHeaderLength = kMacAddressLength * 2 + 2;

/* no worry for frame check sequence, because the hardware computes that for
 * us*/
const size_t kEtherChecksumLength = 4;
const size_t kEtherFrameLengthMax = kEtherHeaderLength + kEtherDataLengthMax;

#endif // TCPSTACK_TYPE_H
