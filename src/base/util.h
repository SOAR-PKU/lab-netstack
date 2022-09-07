//
// Created by Chengke Wong on 2019/9/26.
//

#ifndef TCPSTACK_UTIL_H
#define TCPSTACK_UTIL_H

#include <glog/logging.h>
#include <memory>

#define DISALLOW_COPY_AND_ASSIGN(className)                                    \
  className(const className &) = delete;                                       \
  className &operator=(const className &) = delete;

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) (((uint64_t)htonl((x)&0xFFFFFFFF) << 32u) | htonl((x) >> 32u))
#define ntohll(x) (((uint64_t)ntohl((x)&0xFFFFFFFF) << 32u) | ntohl((x) >> 32u))
#endif

#endif // TCPSTACK_UTIL_H
