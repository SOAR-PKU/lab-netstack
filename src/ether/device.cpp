//
// Created by Chengke Wong on 2019/9/27.
//

#include "device.h"
#include "../base/data_reader.h"
#include "../base/data_writer.h"
#include "./type.h"
#include <glog/logging.h>

Device::Device(pcap_t *pcap, const char *device_name)
    : pcap_(pcap), device_name_(device_name), cb_(nullptr),
      buffer_(new char[kEtherFrameLengthMax]),
      mac_address_(MacAddress::fromDeviceName(device_name)),
      ip_address_(IPAddress::fromDeviceName(device_name)) {
  LOG(INFO) << device_name << " " << ip_address_.toString() << " "
            << mac_address_.toString();
}

void Device::setCallback(IDeviceCallback *cb) { cb_ = cb; }

/**
 * @brief Encapsulate and send an Ethernet II frame to dst.
 * @param buf
 * @param len
 * @param dst
 * @see pcap_sendpacket()
 */
void Device::sendFrame(char *buf, size_t len, MacAddress dst) {
  // LAB: insert your code here.

}

/**
 * @brief Epoll callback on frame arrival.
 * Take out all arrived frames on the device and process them expectively.
 * @see pcap_next_ex()
 */
void Device::onReadable() {
  // LAB: insert your code here.

}

/**
 * @brief Unpack a link layer frame and hand the payload to upper layers.
 *
 * @param data
 * @param length
 */
void Device::decodeFrame(char *data, size_t length) {
  if (cb_ == nullptr) {
    return;
  }

  DataReader reader(data, length);

  MacAddress destination(&reader);
  MacAddress source(&reader);

  //  LOG(DEBUG, << source.toString() << " send to "
  //    << destination.toString());

  if (!destination.isBroadcast() && destination != mac_address_) {
    return;
  }

  if (!peer_address_.isSpecified() && !source.isBroadcast()) {
    peer_address_ = source;
    LOG(INFO) << device_name_ << " find peer: " << peer_address_.toString();
  }

  uint16_t type;
  if (!reader.readUInt16(&type)) {
    return;
  }

  switch (type) {
  case 0x0806: // ARP
    LOG(INFO) << "Receive an ARP";
    return;
  case 0x0800: // Ethernet II
    break;
  default:
    LOG(ERROR) << "unknown type in frame: " << type;
    return;
  }

  cb_->onReceive(this, reader.buffer(), reader.length());
}

void Device::sendFrame(char *buf, size_t len) {
  sendFrame(buf, len, peer_address_);
}

Device::~Device() {
  if (pcap_ != nullptr) {
    pcap_close(pcap_);
  }
}

const std::string &Device::getDeviceName() const { return device_name_; }
