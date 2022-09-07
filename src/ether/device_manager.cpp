//
// Created by Chengke Wong on 2019/9/26.
//

#include "device_manager.h"

#include <glog/logging.h>
#include <pcap/pcap.h>

Device *DeviceManager::findDevice(const char *device_name) {
  auto iter = device_list_.find(device_name);
  if (iter != device_list_.end()) {
    return iter->second.get();
  }
  LOG(ERROR) << "cannot find device " << device_name;
  return nullptr;
}

const std::vector<Device *> &DeviceManager::getAllDevices() const {
  return devices_;
}

DeviceManager::DeviceManager(EpollServer *epoll_server)
    : epoll_server_(epoll_server), device_list_(), devices_() {
  pcap_if_t *all_dev;
  char errbuf[PCAP_ERRBUF_SIZE];
  if (pcap_findalldevs(&all_dev, errbuf) != 0) {
    LOG(ERROR) << errbuf;
    return;
  }

  pcap_if_t *device;
  for (device = all_dev; device != nullptr; device = device->next) {
    addDevice(device);
  }

  pcap_freealldevs(all_dev);
}

DeviceManager::~DeviceManager() = default;

Device *DeviceManager::findDefaultDevice() {
  if (device_list_.empty()) {
    return nullptr;
  }

  return device_list_.begin()->second.get();
}

/**
 * @brief Add a new device to device manager, it should performs:
 * 1. Create a pcap_t and activate it.
 * 2. Create a new Device and add it to the device manager.
 * 3. Register this pcap_t to the epoll server so that a frame arrival event can
 * be handled.
 *
 * @param device returned by pcap_findalldevs
 */
void DeviceManager::addDevice(pcap_if_t *device) {
  char errbuf[PCAP_ERRBUF_SIZE];

  // LAB: insert your code here.

}

void DeviceManager::setCallback(IDeviceCallback *callback) {
  for (auto &item : device_list_) {
    Device *device = item.second.get();
    device->setCallback(callback);
  }
}

bool DeviceManager::activateDevice(pcap_t *pcap) {
  char errbuf[PCAP_ERRBUF_SIZE];
  int rv;

  if ((rv = pcap_activate(pcap)) != 0) {
    if (rv < 0) {
      DLOG(INFO) << pcap_statustostr(rv);
      return false;
    }
    LOG(INFO) << pcap_statustostr(rv);
  }

  int link_type = pcap_datalink(pcap);
  if (link_type < 0) {
    LOG(ERROR) << pcap_statustostr(link_type);
    return false;
  }

  if (link_type != DLT_EN10MB) {
    DLOG(INFO) << " not IEEE 802.3 Ethernet";
    return false;
  }

  if ((rv = pcap_setnonblock(pcap, /* nonblock = */ 1, errbuf)) < 0) {
    LOG(ERROR) << pcap_statustostr(rv);
    return false;
  }

  return true;
}

bool DeviceManager::hasIpAddress(IPAddress address) {
  return ip_set_.find(address) != ip_set_.end();
}

void DeviceManager::broadcastFrame(char *buf, size_t len) {
  for (auto &item : device_list_) {
    Device *device = item.second.get();
    device->sendFrame(buf, len);
  }
}
