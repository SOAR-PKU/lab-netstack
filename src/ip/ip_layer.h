//
// Created by Chengke Wong on 2019/9/30.
//

#ifndef SRC_IP_IP_LAYER_H
#define SRC_IP_IP_LAYER_H

#include "../base/rand_generator.h"
#include "../ether/device_manager.h"
#include "ip_address.h"
#include "routing_table.h"

enum class ServiceProtocol {
  ICMP = 1,
  TCP = 6,

  /* RFC 3692: Use for experimentation and testing */
  TESTING0 = 253,
  TESTING1 = 254,
};

/**
 * @brief Network layer callback for handing packets to upper layers.
 * Transport layer should derive and implement this callback interface.
 *
 */
class IPacketCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(IPacketCallback)

  IPacketCallback() = default;
  virtual ~IPacketCallback() = default;

  virtual void onReceivePacket(char *buffer, size_t length, IPAddress source,
                               IPAddress destination) = 0;
};

/**
 * @brief IPLayer implements routing algorithm and packet routing.
 *
 */
class IPLayer : public IDeviceCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(IPLayer)

  explicit IPLayer(DeviceManager *device_manager, AlarmFactory *alarm_factory);
  ~IPLayer() override = default;

  virtual bool sendPacket(IPAddress source, IPAddress destination,
                          ServiceProtocol proto, void *buf, size_t length);

  void setCallback(IPacketCallback *callback);

  IPAddress getDefaultIP();

protected:
  IPLayer();

private:
  static constexpr TimeBase::Delta probeInterval() {
    return TimeBase::Delta::fromSeconds(3);
  }

  void onReceive(Device *device, char *buf, size_t len) override;

  static bool subtractTTL(char *buf, size_t len);

  bool makePacket(IPAddress source, IPAddress destination,
                  ServiceProtocol proto, void *buf, size_t length);

  bool deliverPacket(const IPAddress &destination, size_t packet_length);

  bool routePacket(const IPAddress &dest, char *buf, size_t len);

  void processProbePacket(const IPAddress &src, const IPAddress &dest,
                          char *buf, size_t len, Device *device);

  void sendProbePacket();

  DEFINE_ALARM_DELEGATE(ProbeAlarmDelegate, IPLayer, sendProbePacket);

  DeviceManager *device_manager_;
  RoutingTable routing_table_;
  std::unique_ptr<char[]> buffer_;
  IPacketCallback *callback_;
  AlarmFactory *alarm_factory_;
  Alarm *probe_alarm_;
  RandGenerator rand_generator_;
};

#endif // SRC_IP_IP_LAYER_H
