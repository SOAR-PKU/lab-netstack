//
// Created by Chengke Wong on 2019/10/1.
//

#include "ip_address.h"
#include "gtest/gtest.h"

class IpAddressTest : public ::testing::Test {
protected:
  IpAddressTest() = default;
  ~IpAddressTest() override = default;

  void SetUp() override {
    char ip[] = {0x10, 0x20, 0x30, 0x40};
    DataReader reader(ip, 4);
    ip_address_ = new IPAddress(&reader);
    EXPECT_EQ(reader.length(), 0);
  }

  void TearDown() override { delete ip_address_; };

  IPAddress *ip_address_;
};

TEST_F(IpAddressTest, ByteOrder) {
  char buf[4];
  DataWriter writer(buf, 4);
  EXPECT_TRUE(ip_address_->writeTo(&writer));
  EXPECT_EQ(buf[0], 0x10);
}

TEST_F(IpAddressTest, ToString) {
  char buffer[4];
  std::string str = ip_address_->toString();
  EXPECT_EQ(str, "16.32.48.64");
}