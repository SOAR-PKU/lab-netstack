/**
 * @file routing_table_test.cpp
 * @brief This testsuit tests IP layer routing table behaviors.
 * DO NOT MODIFY THIS FILE!
 *
 */

#include "routing_table.h"

#include "../util/mock_alarm.h"
#include "gtest/gtest.h"

class MockDevice : public Device {
public:
  explicit MockDevice(int num, std::string name)
      : Device(), num_(num), name_(name) {}

  int num() const { return num_; }

  const std::string &getDeviceName() const { return name_; }

private:
  int num_;
  std::string name_;
};

class RoutingTableTest : public ::testing::Test {
protected:
  MockDevice d1;
  MockDevice d2;
  MockDevice d3;

  RoutingTableTest()
      : d1(1, std::string("dev1")), d2(2, std::string("dev2")),
        d3(3, std::string("dev3")), table(nullptr) {}

  ~RoutingTableTest() override = default;

  std::unique_ptr<RoutingTable> table;
  std::unique_ptr<MockAlarmFactory> time_machine;

  void SetUp() override {
    time_machine = std::make_unique<MockAlarmFactory>();
    table = std::make_unique<RoutingTable>(time_machine.get());
  }

  void TearDown() override{};
};

TEST_F(RoutingTableTest, SimpleMatch) {
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 0, true);
  table->addEntry("192.168.0.233", "255.255.255.255", &d2, 0, true);
  MockDevice *rv;
  RoutingTable::Entry *entry;
  table->printTable();

  entry = table->lookup("192.168.0.233");
  ASSERT_NE(entry, nullptr);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 2);

  entry = table->lookup("192.168.0.1");
  EXPECT_EQ(entry, nullptr);
}

TEST_F(RoutingTableTest, LongestPrefix) {
  table->addEntry("192.168.0.123", "255.255.255.0", &d2, 0, true);
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 0, true);
  table->addEntry("192.168.0.123", "255.255.255.254", &d3, 0, true);
  MockDevice *rv;
  RoutingTable::Entry *entry;

  entry = table->lookup("192.168.0.123");
  ASSERT_NE(entry, nullptr);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 1);
}

TEST_F(RoutingTableTest, DVCalculation) {
  MockDevice *rv;
  RoutingTable::Entry *entry;
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 3, false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry->metric, 4);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 1);

  table->addEntry("192.168.0.123", "255.255.255.255", &d2, 2, false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry->metric, 3);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 2);

  table->addEntry("192.168.0.123", "255.255.255.255", &d2,
                  RoutingTable::Entry::Infinity(), false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry, nullptr);

  table->addEntry("192.168.0.123", "255.255.255.255", &d3, 1, false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry->metric, 2);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 3);
}

TEST_F(RoutingTableTest, EntryTimeout) {
  MockDevice *rv;
  RoutingTable::Entry *entry;
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 3, false);
  table->addEntry("192.168.0.124", "255.255.255.255", &d2, 4, false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry->metric, 4);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 1);

  time_machine->elapse(TimeBase::Delta::fromSeconds(10));
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry, nullptr);
  entry = table->lookup("192.168.0.124");
  EXPECT_EQ(entry, nullptr);
}

TEST_F(RoutingTableTest, EntryKeepAlive) {
  MockDevice *rv;
  RoutingTable::Entry *entry;
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 3, false);
  entry = table->lookup("192.168.0.123");
  EXPECT_EQ(entry->metric, 4);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 1);

  time_machine->elapse(TimeBase::Delta::fromSeconds(3));
  table->addEntry("192.168.0.124", "255.255.255.255", &d1, 4, false);

  time_machine->elapse(TimeBase::Delta::fromSeconds(4));
  entry = table->lookup("192.168.0.123");
  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->metric, 4);
}

TEST_F(RoutingTableTest, CreateDisVec) {
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 0, true);
  table->addEntry("192.168.0.124", "255.255.255.255", &d2, 1, true);
  table->addEntry("192.168.0.125", "255.255.255.255", &d3, 1, true);

  auto dist_vec = table->createDistVec(&d2);
  EXPECT_EQ(dist_vec->len,
            sizeof(uint8_t) + 2 * (sizeof(in_addr::s_addr) + sizeof(uint8_t)));
}

TEST_F(RoutingTableTest, UpdateTable) {
  table->addEntry("192.168.0.123", "255.255.255.255", &d1, 0, true);
  table->addEntry("192.168.0.124", "255.255.255.255", &d2, 1, true);
  table->addEntry("192.168.0.125", "255.255.255.255", &d3, 1, true);

  auto dist_vec = table->createDistVec(&d2);
  table->Clear();
  table->updateTable(dist_vec->buf.get(), dist_vec->len, &d2);

  MockDevice *rv;
  RoutingTable::Entry *entry;
  entry = table->lookup("192.168.0.123");
  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->metric, 1);
  rv = dynamic_cast<MockDevice *>(entry->device);
  EXPECT_EQ(rv->num(), 2);
}