//
// Created by Chengke Wong on 2019/10/23.
//

#include "rand_generator.h"

RandGenerator::RandGenerator(uint32_t seed) : seed_(seed), rand_(seed) {}

uint32_t RandGenerator::rand() {
  /** reference :
   * https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use**/
  // Note that this will generate cyclic mod n sequences of {0, 1 ,..., n-1}
  // if modded with a power of 2
  //  rand_ = rand_ * 1103515245 + 12345;
  //  rand_ &= ~(1u << 31u);

  // rand_ = rand_ * 10086 + 10000007; // ill parameters: always get odd numbers

  /** https://en.wikipedia.org/wiki/Lehmer_random_number_generator **/
  // lcg_parkmiller

  uint32_t low = (rand_ & 0x7fffu) * 48271;
  uint32_t high = (rand_ >> 15u) * 48271;
  uint32_t x = low + ((high & 0xffffu) << 15u) + (high >> 16u);
  rand_ = (x & 0x7fffffffu) + (x >> 31u);
  return rand_;
}

uint32_t RandGenerator::rand(uint32_t lower, uint32_t upper) {
  DCHECK(lower < upper);
  return lower + rand() % (upper - lower);
}

uint32_t RandGenerator::rand(uint32_t upper) { return rand(0, upper); }
