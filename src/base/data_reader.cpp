//
// Created by Chengke Wong on 2019/9/30.
//

#include "data_reader.h"
#include <cstring>
#include <netinet/in.h>

DataReader::DataReader(char *buf, size_t capacity)
    : buffer_(buf), capacity_(capacity), pos_(0) {}

bool DataReader::readUInt8(uint8_t *result) {
  uint8_t value;
  if (!readBytes(&value, sizeof(value))) {
    return false;
  }
  *result = value;

  return true;
}

bool DataReader::readUInt16(uint16_t *result) {
  uint16_t value;
  if (!readBytes(&value, sizeof(value))) {
    return false;
  }
  *result = ntohs(value);

  return true;
}

bool DataReader::readUInt32(uint32_t *result) {
  uint32_t value;
  if (!readBytes(&value, sizeof(value))) {
    return false;
  }
  *result = ntohl(value);

  return true;
}

bool DataReader::readUInt64(uint64_t *result) {
  uint64_t value;
  if (!readBytes(&value, sizeof(value))) {
    return false;
  }
  *result = ntohll(value);

  return true;
}

bool DataReader::skip(size_t length) {
  if (pos_ + length > capacity_) {
    return false;
  }
  pos_ += length;
  return true;
}

bool DataReader::readBytes(void *result, size_t length) {
  if (pos_ + length > capacity_) {
    return false;
  }

  memcpy(result, buffer_ + pos_, length);
  pos_ += length;

  return true;
}

void DataReader::reset() { pos_ = 0; }
