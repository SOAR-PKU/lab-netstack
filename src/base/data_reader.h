//
// Created by Chengke Wong on 2019/9/30.
//

#ifndef SRC_BASE_DATA_READER_H
#define SRC_BASE_DATA_READER_H

#include "util.h"

class DataReader {
public:
  DISALLOW_COPY_AND_ASSIGN(DataReader)
  DataReader(char *buf, size_t capacity);

  bool readUInt8(uint8_t *result);
  bool readUInt16(uint16_t *result);
  bool readUInt32(uint32_t *result);
  bool readUInt64(uint64_t *result);
  bool readBytes(void *result, size_t length);

  bool skip(size_t length);

  void reset();

  inline char *buffer() { return buffer_ + pos_; }

  inline size_t length() { return capacity_ - pos_; }

private:
  char *buffer_;
  size_t capacity_;
  size_t pos_;
};

#endif // SRC_BASE_DATA_READER_H
