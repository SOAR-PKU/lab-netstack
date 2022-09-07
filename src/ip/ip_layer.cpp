//
// Created by Chengke Wong on 2019/9/30.
// Modified by Yuhan Zhou on 2022/7/15.
//

#include "ip_layer.h"

#include "../base/checksum.h"
#include "../base/util.h"

/**
 *
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version|  IHL  |Type of Service|          Total Length         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Identification        |Flags|      Fragment Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Time to Live |    Protocol   |         Header Checksum       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination Address                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                    Example Internet Datagram Header
 */

/**
 * @brief Encapsulate an IP packet and send it.
 *
 * @param source
 * @param destination
 * @param proto
 * @param buf
 * @param length
 * @return true on success, false on failure.
 */
bool IPLayer::sendPacket(IPAddress source, IPAddress destination,
                         ServiceProtocol proto, void *buf, size_t length) {
  if (!makePacket(source, destination, proto, buf, length)) {
    return false;
  }
  size_t packet_length = kPacketHeaderLength + length;
  return deliverPacket(destination, packet_length);
}

/**
 * @brief Look up routing table and send packet stored in class member
 * `buffer_`.
 *
 * @param destination
 * @param packet_length
 * @return true on success, false on failure
 */
bool IPLayer::deliverPacket(const IPAddress &destination,
                            size_t packet_length) {
  // LAB: insert your code here.

  return true;
}

void IPLayer::setCallback(IPacketCallback *callback) { callback_ = callback; }

IPLayer::IPLayer(DeviceManager *device_manager, AlarmFactory *alarm_factory)
    : device_manager_(device_manager), routing_table_(alarm_factory),
      buffer_(new char[kMaxIpPacketLength]), callback_(nullptr),
      alarm_factory_(alarm_factory),
      probe_alarm_(alarm_factory->createAlarm(
          std::make_unique<ProbeAlarmDelegate>(this))),
      rand_generator_(233) {

  // add self IPs to routing talbe so that they appear in DVs
  // these entries will never be updated or marked as invalid
  auto devices = device_manager_->getAllDevices();
  for (auto device : devices) {
    routing_table_.addEntry(device->getIpAddress(), IPAddress::fullMask(),
                            device, 0, true);
  }
  DLOG(INFO) << "initial table";
  routing_table_.printTable();
  probe_alarm_->set(alarm_factory->now() + probeInterval());
}

/**
 * @brief On receiving an IP packet:
 * 1. update routing table or
 * 2. forward it to other hosts or
 * 3. hand it to upper layers
 *
 * @param device
 * @param buf
 * @param len
 */
void IPLayer::onReceive(Device *device, char *buf, size_t len) {
  DCHECK(len >= kPacketHeaderLength);

  DataReader reader(buf, len);
  uint8_t rv;
  reader.readUInt8(&rv);
  if ((rv >> 4u) != 0x4) { // version 4
    LOG(INFO) << "unsupported IP version " << (rv >> 4u);
    return;
  }

  size_t header_length = (rv & 0xfu) * 4;

  reader.skip(1); // type of service
  uint16_t total_length;
  DCHECK(reader.readUInt16(&total_length));
  reader.skip(5);
  ServiceProtocol protocol;
  reader.readUInt8(&rv);
  protocol = (ServiceProtocol)rv;
  reader.skip(2); // checksum
  IPAddress source(&reader);
  IPAddress destination(&reader);

  // skip options

  if (!subtractTTL(buf, len)) {
    LOG(INFO) << "TTL = 0 " << destination.toString();
    return;
  }

  if (protocol == ServiceProtocol::TESTING0) {
    // LAB: insert your code here.

  }

  if (!device_manager_->hasIpAddress(destination)) {
    // LAB: insert your code here.

  }

  switch (protocol) {
    // LAB: insert your code here.

  }
}

bool IPLayer::routePacket(const IPAddress &dest, char *buf, size_t len) {
  DataWriter data_writer(buffer_.get(), kMaxIpPacketLength);
  bool rv = data_writer.writeBytes(buf, len);
  DCHECK(rv);
  return deliverPacket(dest, len);
}

/**
 * @brief On receiving a routing probe (DV) packet, process it and update
 * routing table if possible.
 *
 * @param src
 * @param dest
 * @param buf
 * @param len
 * @param device
 */
void IPLayer::processProbePacket(const IPAddress &src, const IPAddress &dest,
                                 char *buf, size_t len, Device *device) {

  if (device_manager_->hasIpAddress(dest)) {
    return;
  }

  // parse DV and let routing table handle it
  routing_table_.updateTable(buf, len, device);
}

bool IPLayer::subtractTTL(char *buf, size_t len) {
  auto *ptr = (uint8_t *)(buf + kTTLHeaderOffset);
  if (*ptr == 0) {
    return false;
  }
  *ptr -= 1;
  return true;
}

/**
 * @brief Periodically send routing probe (DV) packets to all neighbours.
 *
 */
void IPLayer::sendProbePacket() {
  auto devices = device_manager_->getAllDevices();
  for (auto device : devices) {
    // LAB: insert your code here.

  }

  DLOG(INFO) << "sending routing table";

  probe_alarm_->update(alarm_factory_->now() + probeInterval());
  return;
}

/**
 * @brief Encapsulate an IP packet and store it in class member `buffer_`
 *
 * @param source
 * @param destination
 * @param proto
 * @param buf
 * @param length
 * @return true on success, false on failure
 */
bool IPLayer::makePacket(IPAddress source, IPAddress destination,
                         ServiceProtocol proto, void *buf, size_t length) {
  size_t packet_length = kPacketHeaderLength + length;

  DataWriter data_writer(buffer_.get(), kMaxIpPacketLength);
  data_writer.writeUInt8(0x45); // version & header length
  data_writer.writeUInt8(0);    // type of service
  data_writer.writeUInt16(packet_length);

  // LAB: insert your code here.


  return true;
}

IPAddress IPLayer::getDefaultIP() {
  return device_manager_->findDefaultDevice()->getIpAddress();
}

IPLayer::IPLayer() : rand_generator_(0), routing_table_(nullptr) {}
