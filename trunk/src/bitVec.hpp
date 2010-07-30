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

static const uint64_t L_SHIFT = 9;
static const uint64_t L_BLOCK = 1LLU << L_SHIFT;
static const uint64_t S_SHIFT = 6;
static const uint64_t S_BLOCK = 1LLU << S_SHIFT;
static const uint64_t S_RATIO = L_BLOCK / S_BLOCK;


struct BitVec {
  BitVec();
  ~BitVec();

  void push_back(const uint8_t b);
  void push_back(const uint64_t x, const uint64_t len);

  void setBit(const uint64_t pos, const uint8_t b);
  uint8_t getBit(const uint64_t pos) const;
  uint64_t getBits(const uint64_t pos, const uint64_t len) const;
  void save(std::ostream& os); // non const because call vacuum
  void load(std::istream& is);

  void print() const;
  size_t getAllocSize() const;
  static uint64_t mask(uint64_t x, uint64_t pos);

  uint64_t lookupBlock(const size_t ind) const;

  size_t size;
  std::vector<uint64_t> B;
};

class RSDic {
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
  RSDic();
  ~RSDic();

  void build(BitVec& bv);
  uint64_t rank(const uint64_t pos, const uint8_t b) const;
  uint64_t select(const uint64_t pos, const uint8_t b) const;

  void save(std::ostream& os); // non const because call vacuum
  void load(std::istream& is);
  void print() const;
  size_t getAllocSize() const;

  static uint64_t popCount(uint64_t r);
  static uint64_t selectBlock(uint64_t pos, uint64_t x, uint8_t b);
  uint64_t selectOverL(const uint64_t pos, const uint8_t b, uint64_t& retPos) const;
  uint8_t getBit(const uint64_t pos) const;

  void insert(const uint64_t pos, const uint8_t b);
  void vacuum();
  size_t size() const;

private:
  uint64_t buildInternal(const uint64_t left, const uint64_t right);
  uint64_t rank1(const uint64_t pos) const; 
  
  void vacuumInternal(const uint64_t nodeID, BitVec& newB) const;

  static uint64_t lg2(const uint64_t x);
  static uint64_t popCountMask(uint64_t x, uint64_t pos);

  static uint64_t getNum(uint64_t oneNum, uint64_t num, uint8_t b);

  BitVec B_;
  std::vector<Node> nodes_;
  size_t size_;
};

#endif // BIT_VEC_HPP__
