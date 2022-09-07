//
// Created by Chengke on 2019/10/26.
//

#ifndef TCPSTACK_SEGMENTFACTORY_H
#define TCPSTACK_SEGMENTFACTORY_H

#include "../base/util.h"
#include "../ip/ip_address.h"
#include "segment.h"
#include "socket_address.h"

/**
 * @brief A TCP segment factory is responsable for generating segments for
 * a specific TCP connection (i.e. <local ip, local port, remote ip, remote
 * port> is dispatched).
 * This factory should be owned by a SocketSession to generate segments
 * conveniently.
 */
class SegmentFactory {
public:
  DISALLOW_COPY_AND_ASSIGN(SegmentFactory)

  SegmentFactory(SocketAddress local, SocketAddress remote);

  std::unique_ptr<Segment> createSegment(SequenceNumber seq, SegmentFlags ctl) {
    DCHECK((ctl & Segment::ACK) == 0);
    return createSegment(seq, 0, ctl);
  }

  std::unique_ptr<Segment> createSegment(SequenceNumber seq, SequenceNumber ack,
                                         SegmentFlags ctl) {
    return createSegment(seq, ack, ctl, nullptr, 0);
  }

  std::unique_ptr<Segment> createSegment(SequenceNumber seq, SequenceNumber ack,
                                         SegmentFlags ctl, char *data,
                                         size_t data_length);

  inline SocketAddress getLocalAddress() const { return local_address_; }

  inline SocketAddress getRemoteAddress() const { return remote_address_; }

private:
  SocketAddress local_address_;
  SocketAddress remote_address_;
};

#endif // TCPSTACK_SEGMENTFACTORY_H
