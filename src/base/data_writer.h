//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef TCPSTACK_DATA_WRITER_H
#define TCPSTACK_DATA_WRITER_H

#include "util.h"

class DataWriter {
public:
  DISALLOW_COPY_AND_ASSIGN(DataWriter)

  DataWriter() = delete;
  DataWriter(char *buffer, size_t length);

  bool writeUInt8(uint8_t value);
  bool writeUInt16(uint16_t value);
  bool writeUInt32(uint32_t value);
  bool writeUInt64(uint64_t value);
  bool writeRandomBytes(size_t length);

  bool writeBytes(void *data, size_t length);

  inline char *begin() { return buffer_; }
  inline char *end() { return buffer_ + length_; }
  inline size_t bytesWritten() { return length_; }

private:
  char *buffer_;
  size_t capacity_;
  size_t length_;
};

#endif // TCPSTACK_DATA_WRITER_H
