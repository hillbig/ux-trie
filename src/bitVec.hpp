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
    L_SHIFT = 9,
    L_BLOCK = 1LLU << L_SHIFT,
    S_SHIFT = 6,
    S_BLOCK = 1LLU << S_SHIFT,
    S_RATIO = L_BLOCK / S_BLOCK
  };

  struct Node{
    Node () : size(0), ones(0), left(0), right(0) {}
    uint64_t size;
    uint64_t ones;
    uint64_t left;
    uint64_t right;

    bool isLeaf() const{
      return (right == 0);
    }
  };

public:
  BitVec();
  ~BitVec();

  void push_back(const uint8_t b);
  void push_back(const uint64_t x, const uint64_t len);
  void build();
  uint64_t rank(const uint64_t pos, const uint8_t b) const;
  uint64_t select(const uint64_t pos, const uint8_t b) const;
  uint8_t getBit(const uint64_t pos) const;
  uint64_t getBits(const uint64_t pos, const uint64_t len) const;
  size_t size() const;
  void save(std::ofstream& ofs); // non const because call vacuum
  void load(std::ifstream& ifs);
  void print() const;
  size_t getAllocSize() const;

  static uint64_t popCount(uint64_t r);
  static uint64_t selectBlock(uint64_t pos, uint64_t x, uint8_t b);
  uint64_t selectOverL(const uint64_t pos, const uint8_t b, uint64_t& retPos) const;

  void vacuum();

private:
  uint64_t buildInternal(const uint64_t left, const uint64_t right);
  uint64_t rank1(const uint64_t pos) const; 
  
  void vacuumInternal(const uint64_t nodeID, std::vector<uint64_t>& newB, size_t offset) const;

  static uint64_t lg2(const uint64_t x);
  static uint64_t popCountMask(uint64_t x, uint64_t pos);
  static uint64_t mask(uint64_t x, uint64_t pos);
  static uint64_t getNum(uint64_t oneNum, uint64_t num, uint8_t b);

  uint64_t size_;

  std::vector<uint64_t> B;
  std::vector<Node> nodes;
};

#endif // BIT_VEC_HPP__
