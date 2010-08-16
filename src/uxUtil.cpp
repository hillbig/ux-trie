/* 
 *  Copyright (c) 2010 Daisuke Okanohara
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include "uxUtil.hpp"

namespace ux_tool{

uint64_t lg2(const uint64_t x){
  uint64_t ret = 0;
  while (x >> ret){
    ++ret;
  }
  return ret;
}
  
uint64_t mask(uint64_t x, uint64_t pos){
  return x & ((1LLU << pos) - 1);
}

uint64_t popCount(uint64_t r) {
  r = (r & 0x5555555555555555ULL) +
    ((r >> 1) & 0x5555555555555555ULL);
  r = (r & 0x3333333333333333ULL) +
    ((r >> 2) & 0x3333333333333333ULL);
  r = (r + (r >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
  r = r + (r >>  8);
  r = r + (r >> 16);
  r = r + (r >> 32);
  return (uint64_t)(r & 0x7f);
}

uint64_t popCountMasked(uint64_t x, uint64_t pos){
  return popCount(mask(x, pos));
}

uint64_t selectBlock(uint64_t pos, uint64_t block, uint8_t b) {
  uint64_t ret = 0;
  for (uint64_t t = 32; t >= 8 && pos >= 0; t /= 2){
    uint64_t rankVal = getBitNum(popCountMasked(block, t), t, b);
    if (rankVal >= pos) break;
    block >>= t;
    ret += t;
    pos -= rankVal;
  }

  while (pos > 0){
    if ((block & 1LLU) == b){
      pos--;
    }
    block >>= 1;
    ret++;
  }
  return ret-1;
}

uint64_t getBitNum(uint64_t oneNum, uint64_t num, uint8_t bit){
   if (bit) return oneNum;
   else     return num - oneNum;
}

}

