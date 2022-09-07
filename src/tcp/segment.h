//
// Created by Chengke Wong on 2019/9/28.
//

#ifndef SRC_TCP_SEGMENT_H
#define SRC_TCP_SEGMENT_H

#include "../base/data_writer.h"
#include "../ip/ip_address.h"
#include "socket_address.h"
#include "type.h"

/**
 *   TCP Header Format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                            TCP Header Format

          Note that one tick mark represents one bit position.

                               Figure 3.
 */

typedef uint8_t SegmentFlags;

/**
 * @brief Abstraction of a TCP segment.
 * Note that |data_| is just a raw pointer, which means the segment
 * may not own the payload. Use |copyData()| to copy an owned payload.
 *
 */
class Segment {
public:
  Segment() = default;
  ~Segment();

  bool writeSegmentTo(DataWriter *writer);
  bool writeDataTo(DataWriter *writer);

  inline bool isACK() const { return flags_ & ACK; }
  inline bool isRST() const { return flags_ & RST; }
  inline bool isFIN() const { return flags_ & FIN; }
  inline bool isSYN() const { return flags_ & SYN; }

  // Note that we don't copy the data in |buf|.
  static std::unique_ptr<Segment> parse(IPAddress source, IPAddress destination,
                                        char *buf, size_t len);

  // Copy |data_| if we don't own it.
  void copyData();

  enum {
    FIN = 0x01,
    SYN = 0x02,
    RST = 0x04,
    PSH = 0x08,
    ACK = 0x10,
    URG = 0x20,
  };

  SocketAddress source_;
  SocketAddress destination_;
  SequenceNumber sequence_;
  SequenceNumber acknowledgment_;

  SegmentFlags flags_;

  WindowSize window_;

  char *data_;
  bool own_data_;
  size_t data_length_;

  std::string toString() const;
};

std::ostream &operator<<(std::ostream &stream, const Segment &seg);

#endif // SRC_TCP_SEGMENT_H
