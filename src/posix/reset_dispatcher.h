//
// Created by Chengke Wong on 2019/11/4.
//

#ifndef SRC_POSIX_RESET_DISPATCHER_H
#define SRC_POSIX_RESET_DISPATCHER_H

#include "../base/util.h"
#include "../ip/ip_layer.h"

/**
 * @brief ResetDispatcher is an IPacketCallback that handles
 * non-existing TCP sessions.
 *
 */
class ResetDispatcher : public IPacketCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(ResetDispatcher)
  ResetDispatcher(IPLayer *ip_layer, AlarmFactory *alarm_factory);

  void onReceivePacket(char *buffer, size_t length, IPAddress source,
                       IPAddress destination) override;

private:
  IPLayer *ip_layer_;
  AlarmFactory *alarm_factory_;

  std::unique_ptr<char[]> buffer_;
};

#endif // SRC_POSIX_RESET_DISPATCHER_H
