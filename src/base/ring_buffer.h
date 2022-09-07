//
// Created by Chengke on 2019/10/27.
//

#ifndef TCPSTACK_RING_BUFFER_H
#define TCPSTACK_RING_BUFFER_H

#include "util.h"

class RingBuffer {
public:
  DISALLOW_COPY_AND_ASSIGN(RingBuffer)

  explicit RingBuffer(size_t size);
  ~RingBuffer();

  bool empty() { return begin_ == end_; }
  bool full() { return begin_ + size_ == end_; }

  size_t remaining() { return end_ - begin_; }
  size_t free() { return size_ - remaining(); }

  size_t read(char *buffer, size_t length);
  size_t write(const char *buffer, size_t length);

  // Read but do not consume the data.
  size_t read_offset(size_t offset, char *buffer, size_t length);
  size_t consume(size_t length);

private:
  size_t size_;
  // owned
  char *buffer_;

  size_t begin_;
  size_t end_;
};

#endif // TCPSTACK_RING_BUFFER_H
