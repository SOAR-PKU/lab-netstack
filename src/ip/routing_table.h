//
// Created by Chengke Wong on 2019/9/30.
// Modified by Yuhan Zhou on 2022/7/15.
//

#ifndef SRC_IP_ROUTING_TABLE_H
#define SRC_IP_ROUTING_TABLE_H

#include <vector>

#include "../base/alarm.h"
#include "../base/alarm_factory.h"
#include "../base/util.h"
#include "../ether/device_manager.h"
#include "../ether/mac_address.h"
#include "ip_address.h"

class RoutingTable {
public:
  /**
   * @brief Routing table entry provides routing information.
   * An entry can be static or dynamic. Static means that the entry will never
   * be updated and it is used for self routing. Dynamic means that the entry
   * can be updated by a received DV or revoked by TTL expiration.
   *
   */
  class Entry {
  public:
    IPAddress dest;
    IPAddress mask;
    Device *device;
    uint8_t metric;

    Entry(IPAddress dst, IPAddress msk, Device *dev, uint8_t mtrc,
          AlarmFactory *table_alarm_fac);
    ~Entry() = default;
    void keepAlive();
    /**
     * RFC 1058 Routing Information Protocol Section 2.1
     * In the existing implementation of RIP,16 is used.
     * This value is normally referred to as "infinity",
     * since it is larger than the largest valid metric.
     */
    static constexpr uint8_t Infinity() { return (uint8_t)16; }

  private:
    AlarmFactory *table_alarm_factory_;
    Alarm *ttl_alarm_;
    void revokeRoute();
    DEFINE_ALARM_DELEGATE(EntryTimeoutAlarmDelegate, Entry, revokeRoute);
    static constexpr TimeBase::Delta validInterval() {
      // Set valid interval to be 2 times of probeInterval
      return TimeBase::Delta::fromSeconds(6);
    }
  };

  /**
   * @brief Ready-to-send distance vector.
   * Distance vector format:
   * element number (u8) | ip address (in_addr_t), distance (u8) | ...
   */
  class DistVec {
  public:
    // length of the whole DV in bytes, not element number
    uint8_t len;
    // ready-to-send DV
    std::unique_ptr<char[]> buf;
    DistVec(uint8_t length);
  };

  DISALLOW_COPY_AND_ASSIGN(RoutingTable);
  RoutingTable(AlarmFactory *alarm_factory);

  void updateTable(char *buf, size_t len, Device *device);

  std::unique_ptr<DistVec> createDistVec(Device *device);

  void addEntry(IPAddress dest, IPAddress mask, Device *device, uint8_t metric,
                bool immortal);

  void Clear();

  Entry *lookup(IPAddress ip_address);

  void printTable();

private:
  // use ptr instead of vector<Entry> to avoid move construction
  std::vector<std::unique_ptr<Entry>> entries_;
  AlarmFactory *alarm_factory_;
};

#endif // SRC_IP_ROUTING_TABLE_H
