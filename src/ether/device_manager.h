//
// Created by Chengke Wong on 2019/9/26.
//

#ifndef TCPSTACK_DEVICE_MANAGER_H
#define TCPSTACK_DEVICE_MANAGER_H

#include "../base/epoll_server.h"
#include "../base/time_base.h"
#include "../base/util.h"
#include "device.h"
#include <bits/unordered_set.h>
#include <pcap.h>
#include <unordered_map>
#include <vector>

/**
 * @brief DeviceManager manages all Devices.
 * It provides a comprehensive device abstraction to upper layers.
 */
class DeviceManager {
public:
  DISALLOW_COPY_AND_ASSIGN(DeviceManager)
  explicit DeviceManager(EpollServer *epoll_server);
  ~DeviceManager();

  typedef std::unordered_set<IPAddress, IPAddress::IpAddressHash> IpSet;
  /* return nullptr if not found */
  Device *findDefaultDevice();

  Device *findDevice(const char *device_name);

  const std::vector<Device *> &getAllDevices() const;

  void setCallback(IDeviceCallback *callback);

  bool hasIpAddress(IPAddress address);

  void broadcastFrame(char *buf, size_t len);

  IpSet getAllDevicesIpAddress() { return ip_set_; }

private:
  EpollServer *epoll_server_;
  // for quickly looking up a device
  std::unordered_map<std::string, std::unique_ptr<Device>> device_list_;

  IpSet ip_set_;
  // for enumarating all devices
  std::vector<Device *> devices_;

  void addDevice(pcap_if_t *device);
  bool activateDevice(pcap_t *pcap);
};

#endif // TCPSTACK_DEVICE_MANAGER_H
