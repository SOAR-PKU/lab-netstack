//
// Created by Chengke Wong on 2019/11/5.
//

#ifndef SRC_UTIL_MOCK_IP_LAYER_H
#define SRC_UTIL_MOCK_IP_LAYER_H

#include "../ip/ip_layer.h"
#include "../tcp/segment.h"
#include <deque>

class MockIPLayer : public IPLayer {
public:
  DISALLOW_COPY_AND_ASSIGN(MockIPLayer)

  MockIPLayer();
  const size_t kMaxQueueLength = 16;

  bool sendPacket(IPAddress source, IPAddress destination,
                  ServiceProtocol proto, void *buf, size_t length) override;

  inline std::deque<std::unique_ptr<Segment>> &queue() { return queue_; }

private:
  std::deque<std::unique_ptr<Segment>> queue_;
};

#endif // SRC_UTIL_MOCK_IP_LAYER_H
