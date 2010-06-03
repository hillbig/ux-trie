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
  if (size_ / M_BLOCK >= B.size()) {
    B.resize(size_ / M_BLOCK + 1);
  }



  if (b) {
    B[size_ / M_BLOCK] |= (1LLU << (size_ % M_BLOCK));
  }
  ++size_;
}

void BitVec::build(){
  if (B.size() == 0) return; 
  size_t sum = 0;
  L.resize(size_ / L_BLOCK + 1);
  M.resize(size_ / M_BLOCK + 1);
  for (uint32_t il = 0; il <= size_; il += L_BLOCK){
    L[il/L_BLOCK] = sum;
    for (uint32_t im = 0; im < L_BLOCK && il + im  <= size_; im += M_BLOCK){
	M[(il + im) / M_BLOCK] = sum - L[il/L_BLOCK];
	sum += popCount(B[(il+im)/M_BLOCK]); 
    }
  }
}

uint32_t BitVec::popCount(uint32_t r) {
  r = ((r & 0xAAAAAAAA) >> 1) + (r & 0x55555555);
  r = ((r & 0xCCCCCCCC) >> 2) + (r & 0x33333333);
  r = ((r >> 4) + r) & 0x0F0F0F0F;
  r = (r>>8) + r;
  return ((r>>16) + r) & 0x3F;
}

uint32_t BitVec::rank(const uint32_t pos, const uint8_t b) const{
  uint32_t pos1  = pos+1;
  uint32_t rank1 = L[pos1 >> L_SHIFT] + M[pos1 >> M_SHIFT] + 
    popCount(B[pos >> M_SHIFT] & ((1LLU << (pos1 % M_BLOCK)) - 1));

  if (b == 1) return rank1;
  else        return pos1 - rank1;
}

 uint32_t BitVec::getNum(uint32_t oneNum, uint32_t num, uint8_t b){
   if (b) return oneNum;
   else   return num - oneNum;
 }

uint32_t BitVec::select(const uint32_t pos, const uint8_t b) const{
  uint32_t left  = 0;
  uint32_t right = L.size();
  uint32_t x     = pos;
  while (left < right){
    uint32_t mid = (left + right)/2;
    if (getNum(L[mid], L_BLOCK * mid, b) < x) left  = mid+1;
    else                                      right = mid;
  }
  uint32_t posL  = (left != 0) ? left - 1 : 0;
  uint32_t posM  = posL * M_RATIO + 1;
  x -= getNum(L[posL], L_BLOCK * posL, b);
  while ((posM < M.size()) && (((posM)%M_RATIO) != 0) &&
	 (getNum(M[posM], M_BLOCK*((posM)%M_RATIO), b) < x)){
    ++posM;
  }
  --posM;

  x -= getNum(M[posM], M_BLOCK * (posM % M_RATIO), b);
  uint32_t ret    = posM * M_BLOCK;
  uint32_t curB   = B[posM];
  uint32_t rank16 = getNum(popCount(curB & ((1LLU << 16) - 1)), 16, b);
  if (rank16 < x){
    curB >>= 16;
    ret += 16;
    x -= rank16;
  }

  uint32_t rank8  = getNum(popCount(curB & ((1LLU << 8) - 1)), 8, b);
  if (rank8 < x){
    curB >>= 8;
    ret += 8;
    x -= rank8;
  }

  while (x > 0){
    if ((curB & 1LLU) == b){
      x--;
    }
    curB >>= 1;
    ret++;
  }
  return ret-1;
}

uint8_t BitVec::getBit(const uint32_t pos) const{
  return (B[pos/M_BLOCK] >> (pos % M_BLOCK)) & 1;
}

size_t BitVec::size() const{
  return size_;
}

void BitVec::save(ofstream& ofs) const{
  ofs.write((const char*)&size_, sizeof(size_));
  ofs.write((const char*)&B[0],  sizeof(B[0])*B.size());
  ofs.write((const char*)&L[0],  sizeof(L[0])*L.size());
  ofs.write((const char*)&M[0],  sizeof(M[0])*M.size());
}

void BitVec::load(ifstream& ifs) {
  ifs.read((char*)&size_, sizeof(size_));
  B.resize((size_ + M_BLOCK - 1) / M_BLOCK);
  L.resize(size_ / L_BLOCK + 1);
  M.resize(size_ / M_BLOCK + 1);

  ifs.read((char*)&B[0],  sizeof(B[0])*B.size());
  ifs.read((char*)&L[0],  sizeof(L[0])*L.size());
  ifs.read((char*)&M[0],  sizeof(M[0])*M.size());
}

size_t BitVec::getAllocSize() const {
  return 
    B.size() * sizeof(B[0]) +
    L.size() * sizeof(L[0]) +
    M.size() * sizeof(M[0]);
}
