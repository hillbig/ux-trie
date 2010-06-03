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
    M_SHIFT = 5,
    M_BLOCK = 1LLU << M_SHIFT,
    M_RATIO = L_BLOCK/M_BLOCK
  };


public:
  BitVec();
  ~BitVec();

  void push_back(const uint8_t b);
  void build();
  uint32_t rank(const uint32_t pos, const uint8_t b) const;
  uint32_t select(const uint32_t pos, const uint8_t b) const;
  uint8_t getBit(const uint32_t pos) const;
  size_t size() const;
  void save(std::ofstream& ofs) const;
  void load(std::ifstream& ifs);
  void print() const;
  size_t getAllocSize() const;
  
private:
  static uint32_t popCount(uint32_t r);
  static uint32_t getNum(uint32_t oneNum, uint32_t num, uint8_t b);
  uint32_t size_;
  std::vector<uint32_t> B;
  std::vector<uint32_t> L;
  std::vector<uint8_t>  M;
};

#endif // BIT_VEC_HPP__
