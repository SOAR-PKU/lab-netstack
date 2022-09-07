//
// Created by Chengke Wong on 2019/10/22.
//

#ifndef SRC_TCP_SEGMENT_DISPATCHER_H
#define SRC_TCP_SEGMENT_DISPATCHER_H

#include "../base/util.h"
#include "../ip/ip_layer.h"
#include "../tcp/socket_address.h"
#include "../tcp/socket_session.h"

/**
 * @brief SegmentDispatcher is an IPacketCallback that handles
 * established TCP sessions.
 *
 */
class SegmentDispatcher : public IPacketCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(SegmentDispatcher)

  explicit SegmentDispatcher(IPacketCallback *next);

  void onReceivePacket(char *buffer, size_t length, IPAddress source,
                       IPAddress destination) override;

  void addSession(SocketAddress local, SocketAddress peer,
                  SocketSession *session);

  void removeSession(SocketSession *session);

private:
  IPacketCallback *next_;

  typedef std::tuple<SocketAddress, SocketAddress, SocketSession *>
      EstablishedSocket;
  std::vector<EstablishedSocket> established_;

  SocketSession *lookupSession(const SocketAddress &to,
                               const SocketAddress &from) const;
};

#endif // SRC_TCP_SEGMENT_DISPATCHER_H
