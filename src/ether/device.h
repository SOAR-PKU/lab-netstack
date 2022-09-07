//
// Created by Chengke Wong on 2019-09-05.
//

#ifndef TCPSTACK_DEVICE_H
#define TCPSTACK_DEVICE_H

#include "../base/epoll_server.h"
#include "../base/util.h"
#include "../ip/ip_address.h"
#include "mac_address.h"
#include <cstddef>
#include <cstdint>
#include <pcap/pcap.h>

class Device;

/**
 * @brief Link layer callback for handing packets to upper layers.
 * Network layer should derive and implement this callback interface.
 */
class IDeviceCallback {
public:
  virtual ~IDeviceCallback() = default;
  virtual void onReceive(Device *device, char *buf, size_t len) = 0;
};

/**
 * @brief A device is responsable for managing network addresses and performing
 * link layer frame IO.
 *
 */
class Device : public EpollCallback {
public:
  DISALLOW_COPY_AND_ASSIGN(Device)
  ~Device();
  explicit Device(pcap_t *pcap, const char *device_name);
  void sendFrame(char *buf, size_t len);
  void sendFrame(char *buf, size_t len, MacAddress dst);
  void setCallback(IDeviceCallback *cb);

  void onReadable() override;

  inline IPAddress getIpAddress() { return ip_address_; }

  virtual const std::string &getDeviceName() const;

protected:
  // Only for testing mocks
  Device() : pcap_(nullptr) {}

private:
  void decodeFrame(char *data, size_t length);

  /**
   * In a static network typology where the binding of an IP address and a MAC
   * address is never changed, we can keep these addresses in this class.
   *
   */
  pcap_t *pcap_;
  std::string device_name_;
  IDeviceCallback *cb_;
  std::unique_ptr<char[]> buffer_;
  MacAddress mac_address_;
  MacAddress peer_address_;
  IPAddress ip_address_;
};

#endif // TCPSTACK_DEVICE_H
