//
// Created by Chengke on 2019/10/30.
//

#ifndef TCPSTACK_LISTEN_DISPATCHER_H
#define TCPSTACK_LISTEN_DISPATCHER_H

#include "../base/util.h"
#include "../ip/ip_layer.h"
#include "../tcp/socket_session.h"
#include "segment_dispatcher.h"
#include "socket_struct.h"
#include <condition_variable>

/**
 * @brief ListenDispatcher is an IPacketCallback that handles
 * listening TCP sessions.
 *
 */
class ListenDispatcher : public IPacketCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(ListenDispatcher)

  explicit ListenDispatcher(IPacketCallback *next, IPLayer *ip_layer,
                            AlarmFactory *alarm_factory, RandGenerator *rand);

  void onReceivePacket(char *buffer, size_t length, IPAddress source,
                       IPAddress destination) override;

  void addListener(SocketAddress target, SocketStruct *socket_st);

  void removeSession(SocketStruct *socket_st);

  void setDispatcher(SegmentDispatcher *dispatcher);

private:
  IPacketCallback *next_;
  IPLayer *ip_layer_;
  AlarmFactory *alarm_factory_;
  RandGenerator *rand_;
  SegmentDispatcher *dispatcher_;

  typedef std::tuple<SocketAddress, SocketStruct *> ListeningSocket;
  std::vector<ListeningSocket> vector_;

  SocketStruct *lookupSession(const SocketAddress &to) const;
};

#endif // TCPSTACK_LISTEN_DISPATCHER_H
