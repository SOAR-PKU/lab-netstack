//
// Created by Chengke Wong on 2019/10/23.
//

#ifndef SRC_BASE_RAND_GENERATOR_H
#define SRC_BASE_RAND_GENERATOR_H

#include "util.h"

class RandGenerator {
public:
  DISALLOW_COPY_AND_ASSIGN(RandGenerator)

  RandGenerator() = delete;

  explicit RandGenerator(uint32_t seed);

  uint32_t rand();
  uint32_t rand(uint32_t upper);
  uint32_t rand(uint32_t lower, uint32_t upper);

private:
  uint32_t seed_;
  uint32_t rand_;
};

#endif // SRC_BASE_RAND_GENERATOR_H
