//
// Created by Chengke on 2019/10/27.
//

#include "ring_buffer.h"
#include <cstring>

RingBuffer::~RingBuffer() { delete[] buffer_; }

RingBuffer::RingBuffer(size_t size)
    : buffer_(new char[size]), size_(size), begin_(0), end_(0) {
  DCHECK(buffer_ != nullptr);

  if (size > 1024 * 1024 * 16 /* 16 MB */) {
    LOG(ERROR) << "Ringbuffer is too large";
  }
}

size_t RingBuffer::read(char *buffer, size_t length) {
  if (empty()) {
    return 0;
  }
  length = std::min(length, remaining());
  DCHECK(begin_ < size_);
  if (begin_ + length >= size_) {
    memcpy(buffer, buffer_ + begin_, size_ - begin_);
    memcpy(buffer + (size_ - begin_), buffer_, length - (size_ - begin_));
    begin_ = begin_ + length - size_;
    end_ = end_ - size_;
  } else {
    memcpy(buffer, buffer_ + begin_, length);
    begin_ = begin_ + length;
  }

  return length;
}

size_t RingBuffer::write(const char *buffer, size_t length) {
  if (full()) {
    return 0;
  }
  length = std::min(length, free());
  DCHECK(begin_ < size_);
  if (end_ >= size_) {
    memcpy(buffer_ + end_ - size_, buffer, length);
  } else {
    if (end_ + length >= size_) {
      memcpy(buffer_ + end_, buffer, size_ - end_);
      memcpy(buffer_, buffer + (size_ - end_), length - (size_ - end_));
      end_ = end_ + length;
    } else {
      memcpy(buffer_ + end_, buffer, length);
      end_ = end_ + length;
    }
  }
  return length;
}

size_t RingBuffer::read_offset(size_t offset, char *buffer, size_t length) {
  size_t begin_off = begin_ + offset;
  length = std::min(length, end_ - begin_off);
  if (length == 0) {
    return 0;
  }
  DCHECK(begin_off + length <= end_);

  if (begin_off < size_) {
    if (begin_off + length >= size_) {
      size_t len1 = size_ - begin_off;
      size_t len2 = length - len1;
      memcpy(buffer, buffer_ + begin_off, len1);
      memcpy(buffer + len1, buffer_, len2);
    } else {
      memcpy(buffer, buffer_ + begin_off, length);
    }
  } else {
    memcpy(buffer, buffer_ + begin_off - size_, length);
  }

  return length;
}

size_t RingBuffer::consume(size_t length) {
  length = std::min(length, remaining());
  DCHECK(begin_ < size_);
  if (begin_ + length >= size_) {
    begin_ = begin_ + length - size_;
    end_ = end_ - size_;
  } else {
    begin_ = begin_ + length;
  }

  return length;
}
