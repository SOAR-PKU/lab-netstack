//
// Created by Chengke Wong on 2019/9/27.
//

#include "data_writer.h"
#include <cstring>
#include <netinet/in.h>

DataWriter::DataWriter(char *buffer, size_t length)
    : buffer_(buffer), capacity_(length), length_(0) {}

bool DataWriter::writeBytes(void *data, size_t length) {
  if (length_ + length > capacity_) {
    return false;
  }

  memcpy(buffer_ + length_, data, length);
  length_ += length;
  return true;
}

bool DataWriter::writeUInt8(uint8_t value) {
  return writeBytes(&value, sizeof(value));
}

bool DataWriter::writeUInt16(uint16_t value) {
  value = htons(value);
  return writeBytes(&value, sizeof(value));
}

bool DataWriter::writeUInt32(uint32_t value) {
  value = htonl(value);
  return writeBytes(&value, sizeof(value));
}

bool DataWriter::writeUInt64(uint64_t value) {
  value = htonll(value);
  return writeBytes(&value, sizeof(value));
}

bool DataWriter::writeRandomBytes(size_t length) {
  if (length_ + length > capacity_) {
    return false;
  }
  for (size_t i = 0; i < length; i++) {
    // TODO: random padding
    buffer_[i] = 0x23;
  }
  length_ += length;
  return true;
}
