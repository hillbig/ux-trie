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

#ifndef BIT_VEC_HPP__
#define BIT_VEC_HPP__

#include <stdint.h>
#include <vector>
#include <fstream>

class BitVec{
  enum {
    L_SHIFT = 8,
    L_BLOCK = 1LLU << L_SHIFT,
    S_SHIFT = 6,
    S_BLOCK = 1LLU << S_SHIFT,
    S_RATIO = L_BLOCK / S_BLOCK
  };


public:
  BitVec();
  ~BitVec();

  void push_back(const uint8_t b);
  void build();
  uint64_t rank(const uint64_t pos, const uint8_t b) const;
  uint64_t select(const uint64_t pos, const uint8_t b) const;
  uint8_t getBit(const uint64_t pos) const;
  size_t size() const;
  void save(std::ofstream& ofs) const;
  void load(std::ifstream& ifs);
  void print() const;
  size_t getAllocSize() const;

  static uint64_t popCount(uint64_t r);
  static uint64_t selectBlock(uint64_t pos, uint64_t x, uint8_t b);
  uint64_t selectOverL(const uint64_t pos, const uint8_t b, uint64_t& retPos) const;
private:
  static uint64_t popCountMask(uint64_t x, uint64_t pos);
  static uint64_t mask(uint64_t x, uint64_t pos);
  static uint64_t getNum(uint64_t oneNum, uint64_t num, uint8_t b);



  

  uint64_t size_;
  std::vector<uint64_t> B;
  std::vector<uint64_t> L;
};

#endif // BIT_VEC_HPP__
