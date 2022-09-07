//
// Created by Chengke Wong on 2019/9/30.
// Modified by Yuhan Zhou on 2022/7/15.
//

#include "routing_table.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

RoutingTable::RoutingTable(AlarmFactory *alarm_factory)
    : entries_(), alarm_factory_(alarm_factory) {}

/**
 * @brief Look up an routing table entry following longest prefix matching rule.
 * @return Pointer to a routing table entry, nullptr on failure (can not find
 * entry or metric >= Infinity).
 */
RoutingTable::Entry *RoutingTable::lookup(IPAddress ip_address) {
  Entry *result = nullptr;
  // LAB: insert your code here.

  return result;
}

/**
 * @brief Add an entry to the routing table
 * @param immortal if it is true then this new entry will be static
 * thus it is directly added into the table.
 * Otherwise this new entry will be added following the DV update rule
 * and a timeout alarm is binded to it.
 */
void RoutingTable::addEntry(IPAddress dest, IPAddress mask, Device *device,
                            uint8_t metric, bool immortal) {
  if (immortal) {
    entries_.push_back(
        std::make_unique<Entry>(dest, mask, device, metric, nullptr));
    return;
  }

  /**
   * RFC 1058: Routing Information Protocol v1
   * The method so far only has a way to lower the metric, as the existing
   * metric is kept until a smaller one shows up.  It is possible that the
   * initial estimate might be too low.  Thus, there must be a way to
   * increase the metric.  It turns out to be sufficient to use the
   * following rule: suppose the current route to a destination has metric
   * D and uses gateway G.  If a new set of information arrived from some
   * source other than G, only update the route if the new metric is
   * better than D.  But if a new set of information arrives from G
   * itself, always update D to the new value.
   */
  bool has_entry = false;
  for (auto &entry : entries_) {
    // LAB: insert your code here.

  }

  if (!has_entry) {
    // add a new entry
    // LAB: insert your code here.

  }
}

/**
 * @brief Update routing table based on the received routing packet.
 * @param buf is the received DV
 */
void RoutingTable::updateTable(char *buf, size_t len, Device *device) {
  DataReader reader(buf, len);
  uint8_t length;
  reader.readUInt8(&length);
  DCHECK(len == sizeof(uint8_t) +
                    length * (sizeof(in_addr::s_addr) + sizeof(uint8_t)));

  uint8_t metric;
  for (int i = 0; i < length; ++i) {
    IPAddress dest(&reader);
    reader.readUInt8(&metric);
    addEntry(dest, IPAddress::fullMask(), device, metric, false);
  }
}

/**
 * @brief Create a distance vector based on the routing table.
 * @param device If an entry has the same out device with this device
 * then it would not appear in the DV (split horizon rule).
 */
std::unique_ptr<RoutingTable::DistVec>
RoutingTable::createDistVec(Device *device) {
  auto dist_vec = std::unique_ptr<DistVec>(new DistVec(entries_.size()));
  // LAB: insert your code here.

  return dist_vec;
}

/**
 * @brief Print routing table for debugging
 *
 */
void RoutingTable::printTable() {
#define PRINT_ENTRY(dest, mask, device, metric)                                \
  do {                                                                         \
    std::cout << std::setw(16) << std::left << dest << std::setw(16)           \
              << std::left << mask << std::setw(10) << std::left << device     \
              << std::setw(6) << std::left << metric << std::endl;             \
  } while (false)

  PRINT_ENTRY("DEST", "MASK", "DEVICE", "METRIC");
  for (auto &entry : entries_) {
    PRINT_ENTRY(entry->dest.toString(), entry->mask.toString(),
                entry->device->getDeviceName(), std::to_string(entry->metric));
  }
}

void RoutingTable::Clear() { entries_.clear(); }

/**
 * @brief Entry constructor.
 *
 * @param dst
 * @param msk
 * @param dev
 * @param mtrc
 * @param table_alarm_fac table alarm factory, nullptr if this newly created
 * entry should be immortal.
 */
RoutingTable::Entry::Entry(IPAddress dst, IPAddress msk, Device *dev,
                           uint8_t mtrc, AlarmFactory *table_alarm_fac)
    : dest(dst), mask(msk), device(dev), metric(mtrc),
      table_alarm_factory_(table_alarm_fac) {
  if (table_alarm_factory_) {
    ttl_alarm_ = table_alarm_factory_->createAlarm(
        std::make_unique<EntryTimeoutAlarmDelegate>(this));
    ttl_alarm_->set(table_alarm_factory_->now() + validInterval());
  } else {
    ttl_alarm_ = nullptr;
  }
}

/**
 * @brief Mark an entry as invalid, it is called on an entry TTL expiration.
 */
void RoutingTable::Entry::revokeRoute() {
  metric = Infinity();
  DLOG(INFO) << "revoking " << dest.toString() << " to "
             << std::to_string(metric);
}

/**
 * @brief Keep an entry valid for at least `validInterval'
 */
void RoutingTable::Entry::keepAlive() {
  if (ttl_alarm_) {
    ttl_alarm_->update(table_alarm_factory_->now() + validInterval(),
                       TimeBase::Delta::fromSeconds(1));
  }
}

/**
 * @brief DistVec constructor.
 *
 * @param length element number of the distance vector.
 */
RoutingTable::DistVec::DistVec(uint8_t length) {
  len = sizeof(uint8_t) + length * (sizeof(in_addr::s_addr) + sizeof(uint8_t));
  buf = std::unique_ptr<char[]>(new char[len]);
}
