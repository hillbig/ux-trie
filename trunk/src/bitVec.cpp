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

BitVec::BitVec() : size_(0){
}

BitVec::~BitVec(){
}

void BitVec::print() const {
  for (uint32_t i = 0; i < size_; ++i){
    if (getBit(i)) cout << "1";
    else           cout << "0";
    if ((i+1)%8 == 0){
      cout << " ";
      if ((i+1)%64 == 0) cout << endl;
    } 
  }
}

void BitVec::push_back(const uint8_t b){
  if (size_ / S_BLOCK >= B.size()) {
    B.resize(size_ / S_BLOCK + 1);
  }

  if (b) {
    B[size_ / S_BLOCK] |= (1ULL << (size_ % S_BLOCK));
  }
  ++size_;
}

void BitVec::build(){
  if (B.size() == 0) return; 

  size_t sum = 0;
  L.resize(size_ / L_BLOCK + 1);
  for (uint64_t il = 0; il <= size_; il += L_BLOCK){
    L[il/L_BLOCK] = sum;
    for (uint64_t is = 0; is < L_BLOCK && il + is  <= size_; is += S_BLOCK){
	sum += popCount(B[(il+is)/S_BLOCK]); 
    }
  }
}

uint64_t BitVec::mask(uint64_t x, uint64_t pos){
  return x & ((1LLU << pos) - 1);
}


uint64_t BitVec::popCountMask(uint64_t x, uint64_t pos){
  return popCount(mask(x, pos));
}

uint64_t BitVec::popCount(uint64_t r) {
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

uint64_t BitVec::rank(const uint64_t pos, const uint8_t b) const{
  uint64_t pos1  = pos+1;
  uint64_t rank1 = L[pos1 >> L_SHIFT];
  uint64_t bpos  = (pos1 >> L_SHIFT) << (L_SHIFT - S_SHIFT);
  uint64_t epos  = pos1 >> S_SHIFT; 
  for (uint64_t i = bpos; i < epos; ++i){
    rank1 += popCount(B[i]);
  }
  rank1 += popCountMask(B[epos], pos1 % S_BLOCK);
  
  if (b == 1) return rank1;
  else        return pos1 - rank1;
}

 uint64_t BitVec::getNum(uint64_t oneNum, uint64_t num, uint8_t b){
   if (b) return oneNum;
   else   return num - oneNum;
 }

uint64_t BitVec::selectOverL(const uint64_t pos, const uint8_t b, uint64_t& retPos) const {
  uint64_t left   = 0;
  uint64_t right  = L.size();
  
  retPos = pos;
  while (left < right){
    uint64_t mid = (left + right)/2;
    assert(mid < L.size());
    if (getNum(L[mid], L_BLOCK * mid, b) < retPos) left  = mid+1;
    else                                           right = mid;
  }
  uint64_t posL = (left != 0) ? left - 1 : 0;

  uint64_t posS  = posL * S_RATIO;

  assert(retPos >= getNum(L[posL], L_BLOCK * posL, b));

  retPos -= getNum(L[posL], L_BLOCK * posL, b);
  for (;;posS++){
    if (posS >= B.size()) break;
    uint64_t num = getNum(popCount(B[posS]), S_BLOCK, b);
    if (retPos <= num) break;
    retPos -= num;
  }
  return posS;
}

uint64_t BitVec::selectBlock(uint64_t pos, uint64_t block, uint8_t b) {
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


uint64_t BitVec::select(const uint64_t pos, const uint8_t b) const{
  uint64_t retPos = 0;
  uint64_t posS   = selectOverL(pos, b, retPos);
  return posS * S_BLOCK + selectBlock(retPos, B[posS], b);
}

uint8_t BitVec::getBit(const uint64_t pos) const{
  return (B[pos/S_BLOCK] >> (pos % S_BLOCK)) & 1;
}

size_t BitVec::size() const{
  return size_;
}

void BitVec::save(ofstream& ofs) const{
  ofs.write((const char*)&size_, sizeof(size_));
  ofs.write((const char*)&B[0],  sizeof(B[0])*B.size());
  ofs.write((const char*)&L[0],  sizeof(L[0])*L.size());
}

void BitVec::load(ifstream& ifs) {
  ifs.read((char*)&size_, sizeof(size_));
  B.resize((size_ + S_BLOCK - 1) / S_BLOCK);
  L.resize(size_ / L_BLOCK + 1);

  ifs.read((char*)&B[0],  sizeof(B[0])*B.size());
  ifs.read((char*)&L[0],  sizeof(L[0])*L.size());
}

size_t BitVec::getAllocSize() const {
  return 
    B.size() * sizeof(B[0]) +
    L.size() * sizeof(L[0]);
}
