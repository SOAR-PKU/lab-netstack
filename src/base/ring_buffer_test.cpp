//
// Created by Chengke on 2019/10/28.
//

#include "ring_buffer.h"
#include "gtest/gtest.h"

class RingBufferTest : public testing::Test {
protected:
  const size_t kBufferSize = 5;

  RingBuffer *buffer;

  void SetUp() override { buffer = new RingBuffer(kBufferSize); }

  void TearDown() override { delete buffer; };

  size_t write(const std::string &s) {
    return buffer->write(s.c_str(), s.length());
  }

  void expectWrite(const std::string &s) {
    EXPECT_EQ(buffer->write(s.c_str(), s.length()), s.length());
  }

  std::string read(size_t len) {
    char b[kBufferSize];
    size_t rv = buffer->read(b, len);
    return std::string(b, rv);
  }

  std::string read_offset(size_t offset, size_t len) {
    char b[kBufferSize];
    size_t rv = buffer->read_offset(offset, b, len);
    return std::string(b, rv);
  }
};

TEST_F(RingBufferTest, Basic) {
  EXPECT_TRUE(buffer->empty());
  expectWrite("hello");
  EXPECT_TRUE(buffer->full());
  EXPECT_EQ(read(5), "hello");
}

TEST_F(RingBufferTest, RingReadWrite) {
  expectWrite("aa");
  read(1);
  expectWrite("1234");
  EXPECT_TRUE(buffer->full());
  EXPECT_EQ(read(5), "a1234");
}

TEST_F(RingBufferTest, RingBufferReadOffset) {
  expectWrite("aa");
  read(1);
  expectWrite("1234");
  EXPECT_TRUE(buffer->full());
  EXPECT_EQ(read_offset(0, 5), "a1234");
  EXPECT_EQ(read_offset(0, 6), "a1234");
  EXPECT_EQ(read_offset(1, 4), "1234");
  EXPECT_EQ(read_offset(1, 2), "12");
  EXPECT_EQ(read_offset(2, 3), "234");
  EXPECT_EQ(read_offset(4, 1), "4");
  EXPECT_EQ(read_offset(4, 0), "");
}
