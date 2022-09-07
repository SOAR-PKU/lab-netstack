//
// Created by Chengke Wong on 2019/9/28.
//

#include "segment.h"
#include "../base/checksum.h"
#include "../base/data_reader.h"
#include "../ip/ip_address.h"
#include "../ip/ip_layer.h"

bool Segment::writeSegmentTo(DataWriter *writer) {
  writer->writeUInt16(source_.port());
  writer->writeUInt16(destination_.port());
  writer->writeUInt32(sequence_);
  writer->writeUInt32(acknowledgment_);
  writer->writeUInt8(0x50);
  writer->writeUInt8(flags_);
  writer->writeUInt16(window_);

  // leave the checksum field as 0
  writer->writeUInt16(0);

  // urgent pointer = 0
  writer->writeUInt16(0);
  writer->writeBytes(data_, data_length_);

  uint16_t tcp_length = data_length_ + kTcpHeaderLength;
  Checksum checksum(writer->begin(), writer->end());

  // now add the 96 bit pseudo header
  checksum.addIPAddress(source_.ipAddress());
  checksum.addIPAddress(destination_.ipAddress());
  checksum.addHostUInt16((uint16_t)ServiceProtocol::TCP);
  checksum.addHostUInt16(tcp_length);

  checksum.writeTo(writer->begin() + 16);

  return false;
}

std::unique_ptr<Segment> Segment::parse(IPAddress source, IPAddress destination,
                                        char *buf, size_t len) {
  std::unique_ptr<Segment> rv = std::make_unique<Segment>();
  DataReader reader(buf, len);
  SocketPort source_port;
  SocketPort destination_port;
  reader.readUInt16(&source_port);
  reader.readUInt16(&destination_port);
  rv->source_ = SocketAddress(source, source_port);
  rv->destination_ = SocketAddress(destination, destination_port);
  reader.readUInt32(&rv->sequence_);
  reader.readUInt32(&rv->acknowledgment_);
  uint8_t offset;
  reader.readUInt8(&offset);
  offset = offset >> 2u;
  reader.readUInt8(&rv->flags_);
  DCHECK(reader.readUInt16(&rv->window_));
  rv->own_data_ = false;
  rv->data_ = buf + offset;
  rv->data_length_ = len - offset;
  return rv;
}

Segment::~Segment() {
  if (own_data_) {
    delete[] data_;
  }
}

bool Segment::writeDataTo(DataWriter *writer) {
  if (data_length_ == 0) {
    return true;
  }

  return writer->writeBytes(data_, data_length_);
}

void Segment::copyData() {
  if (own_data_) {
    return;
  }

  if (data_length_ == 0) {
    return;
  }

  char *data = (char *)malloc(data_length_);
  std::copy(data_, data_ + data_length_, data);
  own_data_ = true;
  data_ = data;
}

std::string Segment::toString() const {
  std::stringstream ss;
  ss << "SEQ=" << sequence_;
  if (isACK()) {
    ss << " ACK=" << acknowledgment_;
  }

  if (isSYN()) {
    ss << " SYN";
  }

  if (isRST()) {
    ss << " RST";
  }

  if (isFIN()) {
    ss << " FIN";
  }

  if (data_length_ > 0) {
    ss << " len=" << data_length_;
  }

  return ss.str();
}

std::ostream &operator<<(std::ostream &stream, const Segment &seg) {
  stream << seg.toString();
  return stream;
}
