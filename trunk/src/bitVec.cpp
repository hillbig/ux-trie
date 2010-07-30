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

#include <iostream>
#include <cassert>
#include "bitVec.hpp"

using namespace std;

BitVec::BitVec() : size(0){
}

BitVec::~BitVec(){
}

RSDic::RSDic() : size_(0) {
}

RSDic::~RSDic() {
}

void BitVec::print() const {
  for (uint32_t i = 0; i < size; ++i){
    if (getBit(i)) cout << "1";
    else           cout << "0";
    if ((i+1)%8 == 0){
      cout << " ";
      if ((i+1)%64 == 0) cout << endl;
    } 
  }
}

void BitVec::push_back(const uint8_t b){
  if (size / S_BLOCK >= B.size()) {
    B.resize(size / S_BLOCK + 1);
  }

  if (b) {
    B[size / S_BLOCK] |= (1ULL << (size % S_BLOCK));
  }
  ++size;
}

uint64_t BitVec::lookupBlock(const size_t ind) const{
  return B[ind];
}

void BitVec::setBit(const uint64_t pos, const uint8_t b){
  if (b == 0) return;
  B[pos / S_BLOCK] = 1LLU << (pos % S_BLOCK);
}

void BitVec::push_back(const uint64_t x, const uint64_t len){
  size_t offset = size % S_BLOCK;
  if ((size + len - 1) / S_BLOCK >= B.size()){
    B.resize((size + len - 1) / S_BLOCK + 1);
  }

  B[size / S_BLOCK] |= (x << offset);
  if (offset + len - 1 > S_BLOCK){
    B[size / S_BLOCK + 1] |= (x >> (S_BLOCK - offset));
  } 
  size += len;
}

uint64_t RSDic::lg2(const uint64_t x){
  uint64_t ret = 0;
  while (x >> ret){
    ++ret;
  }
  return ret;
}

uint64_t RSDic::buildInternal(const uint64_t leftBlock, const uint64_t rightBlock){
  uint64_t blockNum = rightBlock - leftBlock;

  if (blockNum == 1){
    Node node;
    node.left = leftBlock;
    for (uint64_t i = leftBlock * S_RATIO; i < rightBlock * S_RATIO && i < B_.B.size(); ++i){
      node.ones += popCount(B_.lookupBlock(i)) ;
    }
    node.size = std::min(size_, rightBlock * L_BLOCK) - leftBlock * L_BLOCK;
    nodes_.push_back(node);
    return nodes_.size()-1;
  } else {
    nodes_.push_back(Node());
    size_t nodeID = nodes_.size()-1;

    uint64_t width   = (1LLU << (lg2(blockNum-1)-1));
    uint64_t left    = buildInternal(leftBlock,       leftBlock+width);
    uint64_t right   = buildInternal(leftBlock+width, rightBlock);
    Node& node       = nodes_[nodeID];
    Node& leftChild  = nodes_[left];
    Node& rightChild = nodes_[right];
    node.left  = left;
    node.right = right;
    node.ones  = leftChild.ones + rightChild.ones;
    node.size  = leftChild.size + rightChild.size;

    return nodeID;
  }
}

void RSDic::build(BitVec& bv){
  size_ = bv.size;
  swap(B_, bv);
  nodes_.clear();
  if (B_.size == 0) return; 
  buildInternal(0, (size_ + L_BLOCK - 1) / L_BLOCK);
}

uint64_t BitVec::mask(uint64_t x, uint64_t pos){
  return x & ((1LLU << pos) - 1);
}


uint64_t RSDic::popCountMask(uint64_t x, uint64_t pos){
  return popCount(BitVec::mask(x, pos));
}

uint64_t RSDic::popCount(uint64_t r) {
  /* type 4 */
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

uint64_t RSDic::rank1(const uint64_t _pos) const{
  if (nodes_.size() == 0) return 0;
  uint64_t ret    = 0;
  uint64_t pos    = _pos;
  for (uint64_t nodeID = 0; ; ){
    const Node& cur = nodes_[nodeID];
    if (!cur.isLeaf()){
      const Node& leftChild = nodes_[cur.left];
      if (leftChild.size < pos){
	nodeID = cur.right;
	ret += leftChild.ones;
	pos -= leftChild.size;
      } else {
	nodeID = cur.left;
      }
    } else {
      uint64_t bpos = cur.left * S_RATIO;
      uint64_t epos = bpos + (pos >> S_SHIFT);
      for (uint64_t i = bpos; i < epos; ++i){
	ret += popCount(B_.lookupBlock(i));
      }
      ret += popCountMask(B_.lookupBlock(epos), pos % S_BLOCK);
      return ret;
    }
  }
}

uint64_t RSDic::rank(const uint64_t pos, const uint8_t b) const{
  uint64_t pos1 = pos+1;
  uint64_t ones = rank1(pos1);
  if (b == 1) return ones;
  else        return pos1 - ones;
}

uint8_t BitVec::getBit(const uint64_t pos) const{
  return (B[pos/S_BLOCK] >> (pos % S_BLOCK)) & 1;
}

uint64_t BitVec::getBits(const uint64_t pos, const uint64_t len) const{
  uint64_t blockInd1    = pos / S_BLOCK;
  uint64_t blockOffset1 = pos % S_BLOCK;
  if (blockOffset1 + len <= S_BLOCK){
    return mask(B[blockInd1] >> blockOffset1, len);
  } else {
    uint64_t blockInd2    = ((pos + len - 1) / S_BLOCK);
    return  mask((B[blockInd1] >> blockOffset1) + (B[blockInd2] << (S_BLOCK - blockOffset1)), len);
  }
}

uint64_t RSDic::getNum(uint64_t oneNum, uint64_t num, uint8_t b){
   if (b) return oneNum;
   else   return num - oneNum;
 }

uint64_t RSDic::select(const uint64_t pos, const uint8_t b) const{
  uint64_t ret    = 0;
  uint64_t remain = pos;
  uint64_t nodeID = 0;
  for (;;){
    const Node& cur = nodes_[nodeID];
    if (!cur.isLeaf()){
      const Node& leftChild = nodes_[cur.left];
      uint64_t val = getNum(leftChild.ones, leftChild.size, b);
      if (val < remain){
	ret    += leftChild.size;
	remain -= val;
	nodeID = cur.right;
      } else {
	nodeID = cur.left;
      }
    } else {
      uint64_t bpos = cur.left * S_RATIO;
      for (; bpos < B_.size; ++bpos){
	uint64_t num = getNum(popCount(B_.lookupBlock(bpos)), S_BLOCK, b);
	if (remain <= num) break;
	ret    += S_BLOCK;
	remain -= num;
      }
      ret += selectBlock(remain, B_.lookupBlock(bpos), b);
      return ret;
    }

  }
}


uint64_t RSDic::selectBlock(uint64_t pos, uint64_t block, uint8_t b) {
  uint64_t ret = 0;
  for (uint64_t t = 32; t >= 8 && pos >= 0; t /= 2){
    uint64_t rankVal = getNum(popCountMask(block, t), t, b);
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
  assert(pos == 0);
  return ret-1;
}

void RSDic::vacuumInternal(const uint64_t nodeID, BitVec& newB) const{
  const Node& cur = nodes_[nodeID];
  if (cur.isLeaf()){
    uint64_t bBeginPos = cur.left * L_BLOCK;
    for (uint64_t i = 0; i < cur.size; ++i){
      newB.push_back(B_.getBit(bBeginPos + i));
    }
  } else {
    vacuumInternal(cur.left,  newB);
    vacuumInternal(cur.right, newB);
  }
}

void RSDic::vacuum(){
  if (nodes_.size() == 0) return;
  BitVec newB;
  vacuumInternal(0, newB);
  build(newB);
}

size_t RSDic::size() const{
  return size_;
}


void BitVec::save(ostream& os) {
  os.write((const char*)&size, sizeof(size));
  os.write((const char*)&B[0],  sizeof(B[0])*B.size());
}

void RSDic::save(ostream& ofs) {
  vacuum();
  B_.save(ofs);
}

void BitVec::load(istream& ifs) {
  ifs.read((char*)&size, sizeof(size));
  B.resize((size + S_BLOCK - 1) / S_BLOCK);
  ifs.read((char*)&B[0],  sizeof(B[0])*B.size());
}

void RSDic::load(istream& ifs) {
  B_.load(ifs);
  build(B_);
}


size_t BitVec::getAllocSize() const {
  return 
    B.size() * sizeof(B[0]);
}

size_t RSDic::getAllocSize() const {
  return B_.getAllocSize() + sizeof(Node) * nodes_.size();
}

uint8_t RSDic::getBit(const uint64_t pos) const{
  return B_.getBit(pos);
}

