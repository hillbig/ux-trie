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

#include <algorithm>
#include <queue>
#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include "ux.hpp"

using namespace std;

namespace ux_tool{

UX::UX() : keyNum_(0), isReady_(false) {
} 

UX::~UX(){
}

struct RangeNode{
  RangeNode(size_t _left, size_t _right) :
    left(_left), right(_right) {}
  size_t left;
  size_t right;
};

void UX::build(vector<string>& wordList){
  sort(wordList.begin(), wordList.end());
  wordList.erase(unique(wordList.begin(), wordList.end()), wordList.end());

  keyNum_ = wordList.size();
  size_t totalLen = 0;
  for (size_t i = 0; i < wordList.size(); ++i){
    totalLen += wordList[i].size();
    
  }

  size_t lcs = 0;
  for (size_t i = 1; i < wordList.size(); ++i){
    string& s1 = wordList[i-1];
    string& s2 = wordList[i];
    size_t j = 0;
    for (; j < s1.size() && j < s2.size() && s1[j] == s2[j]; ++j) {};
    lcs += j;
  }
  
  

  queue<RangeNode> q;
  queue<RangeNode> nextQ;

  if (keyNum_ != 0){
    q.push(RangeNode(0, keyNum_));
  }

  // super root
  loud_.push_back(0);
  loud_.push_back(1);

  size_t nodeNum = 0;
  size_t depth = 0;
  for (;;){
    if (q.empty()){
      swap(q, nextQ);
      ++depth;
      if (q.empty()) break;
    }
    RangeNode& rn = q.front();
    const size_t left  = rn.left;
    const size_t right = rn.right;
    q.pop();
    nodeNum++;

    if (left + 1 == right){   // tail
      loud_.push_back(1);
      terminal_.push_back(1);
      tail_.push_back(1);

      string tail;
      for (size_t i = depth; i < wordList[left].size(); ++i){
	assert(wordList[left].size() > i);
	tail += wordList[left][i];
      }
      vtails_.push_back(tail);
      continue;
    } else {
      tail_.push_back(0);
    }

    assert(wordList.size() > left);
    size_t newLeft = left;
    if (depth == wordList[left].size()){
      terminal_.push_back(1);
      ++newLeft;
    } else {
      terminal_.push_back(0);
    }

    size_t  prev  = newLeft;
    assert(wordList[prev].size() > depth);
    uint8_t prevC = (uint8_t)wordList[prev][depth];
    uint32_t degree = 0;
    for (size_t i = prev+1; ; ++i){
      if (i < right && 
	  prevC == (uint8_t)wordList[i][depth]){
	continue;
      }
      edges_.push_back(prevC);
      loud_.push_back(0);
      degree++;
      nextQ.push(RangeNode(prev, i));
      if (i == right){
	break;
      }
      prev  = i;
      assert(wordList[prev].size() > depth);
      prevC = wordList[prev][depth];

    }
    loud_.push_back(1);
  }
  size_t total = loud_.size() + terminal_.size() + edges_.size();

  size_t tailslen = 0;
  for (size_t i = 0; i < vtails_.size(); ++i){
    tailslen += vtails_[i].size();
  }


  cout << "    loud:\t" << loud_.size()     << endl
       << "terminal:\t" << terminal_.size() << endl
       << "    edge:\t" << edges_.size()    << endl
       << " avgedge:\t" << (float)edges_.size() / keyNum_ << endl
       << "   total:\t" << total << endl
       << "  vtails:\t" << tailslen << endl
       << " avgtail:\t" << (float)tailslen / keyNum_ << endl
       << "totallen:\t" << totalLen << endl
       << "  avelen:\t" << (float)totalLen / keyNum_ << endl
       << "  avelcs:\t" << (float)lcs / keyNum_ << endl;

  loud_.build();
  terminal_.build();
  tail_.build();

  if (keyNum_ > 0){
    isReady_ = true;
  }

}

void UX::getChild(const uint8_t c, uint32_t& pos, uint32_t& zeros) const {
  for (;; ++pos, ++zeros){
    if (loud_.getBit(pos)){
      pos = NOTFOUND;
      return;
    }
    assert(zeros >= 2);
    assert(edges_.size() > zeros-2);
    if (edges_[zeros-2] == c){
      pos   = loud_.select(zeros, 1)+1;
      zeros = pos - zeros + 1;
      return;
    }
  }
}

bool UX::isLeaf(const uint32_t pos) const {
  return loud_.getBit(pos);
}

void UX::getParent(uint8_t& c, uint32_t& pos, uint32_t& zeros) const {
  zeros = pos - zeros + 1;
  pos   = loud_.select(zeros, 0);
  if (zeros < 2) return;
  assert(edges_.size() > zeros-2);
  c     = edges_[zeros-2];
}  

id_t UX::prefixSearch(const char* str, const size_t len, size_t& retLen) const{
  vector<id_t> retIDs;
  traverse(str, len, retLen, retIDs, 0xFFFFFFFF);
  if (retIDs.size() == 0){
    return NOTFOUND;
  }
  return retIDs.back();
}

size_t UX::predictiveSearch(const char* str, const size_t len, vector<id_t>& retIDs, 
			    const size_t limit) const{
  retIDs.clear();
  if (!isReady_) return 0;
  if (limit == 0) return 0;

  uint32_t lastPos   = 0;
  uint32_t lastZeros = 0;
  uint32_t pos       = 2;
  uint32_t zeros     = 2;
  for (size_t i = 0; i < len; ++i){
    lastPos   = pos;
    lastZeros = zeros;
    getChild((uint8_t)str[i], pos, zeros);
    if (pos == NOTFOUND){
      return 0;
    }
  }

  // search all descendant nodes from curPos
  enumerateAll(lastPos, lastZeros, retIDs, limit);
  return retIDs.size();
}

void UX::enumerateAll(const uint32_t pos, const uint32_t zeros, vector<id_t>& retIDs, const size_t limit) const{
  const uint32_t ones = pos - zeros;
  if (terminal_.getBit(ones)){
    retIDs.push_back(terminal_.rank(ones, 1) - 1);
  }
  
  for (uint32_t i = 0; loud_.getBit(pos + i) == 0 &&
	 retIDs.size() < limit; ++i){
    uint32_t nextPos = loud_.select(zeros + i, 1)+1;
    enumerateAll(nextPos, nextPos - zeros - i + 1,  retIDs, limit);
  }
}

size_t UX::commonPrefixSearch(const char* str, const size_t len, vector<id_t>& retIDs,
			      const size_t limit) const {
  retIDs.clear();
  size_t lastLen = 0;
  traverse(str, len, lastLen, retIDs, limit);
  return retIDs.size();
}

void UX::traverse(const char* str, const size_t len, 
		  size_t& lastLen, std::vector<id_t>& retIDs, const size_t limit) const{

  lastLen = 0;
  if (!isReady_) return;
  if (limit == 0) return;

  uint32_t pos   = 2;
  uint32_t zeros = 2;
  for (size_t depth = 0; pos != NOTFOUND; ++depth){
    const uint32_t nodeID = zeros - 2;
    assert(terminal_.size() > nodeID);
    uint32_t ones = pos - zeros;

    if (tail_.getBit(ones)){
      if (tailMatch(str, len, depth, tail_.rank(ones, 1)-1)){
	lastLen = depth + vtails_[tail_.rank(ones, 1)-1].size();
	retIDs.push_back(terminal_.rank(ones, 1) - 1);
      }
      break;
    } else if (terminal_.getBit(ones)){
      lastLen = depth;
      retIDs.push_back(terminal_.rank(ones, 1)-1);
      if (retIDs.size() == limit) {
	break;
      }
    }

    if (depth == len) break;
    
    getChild((uint8_t)str[depth], pos, zeros);
  }
}

bool UX::tailMatch(const char* str, const size_t len, const size_t depth,
		   const uint32_t tailID) const{
  const string& tail(vtails_[tailID]);
  if (tail.size() > len-depth) {
    return false;
  }
  for (size_t i = 0; i < tail.size(); ++i){
    if (str[i+depth] != tail[i]) {
      return false;
    }
  }
  return true;
}

void UX::decode(const id_t id, string& ret) const{
  ret.clear();
  if (!isReady_) return;

  uint32_t nodeID = terminal_.select(id+1, 1);
  
  uint32_t pos    = loud_.select(nodeID+1, 1) + 1;
  uint32_t zeros  = pos - nodeID;
  for (;;) { 
    uint8_t c = 0;
    getParent(c, pos, zeros);
    if (pos == 0) break;
    ret += (char)c;
  }
  reverse(ret.begin(), ret.end());
  if (tail_.getBit(nodeID)){
    ret += vtails_[tail_.rank(nodeID, 1) - 1];
  }
}

string UX::decode(const id_t id) const {
  std::string ret;
  decode(id, ret);
  return ret;
}

size_t UX::getKeyNum() const {
  return keyNum_;
}


int UX::save(const char* fn) const {
  ofstream ofs(fn);
  if (!ofs){
    return FILE_OPEN_ERROR;
  }

  loud_.save(ofs);
  terminal_.save(ofs);
  tail_.save(ofs);

  ofs.write((const char*)&keyNum_, sizeof(keyNum_));
  size_t edgesSize = edges_.size();
  ofs.write((const char*)&edgesSize, sizeof(edgesSize));
  ofs.write((const char*)&edges_[0], sizeof(edges_[0]) * edges_.size()); 
  size_t tailsNum  = vtails_.size();
  ofs.write((const char*)&tailsNum,  sizeof(tailsNum));
  for (size_t i = 0; i < vtails_.size(); ++i){
    size_t tailSize = vtails_[i].size();
    ofs.write((const char*)&tailSize,  sizeof(tailSize));
    ofs.write((const char*)&vtails_[i][0], sizeof(vtails_[i][0]) * vtails_[i].size());
  }

  if (!ofs){
    return FILE_WRITE_ERROR;
  }

  return 0;
}

int UX::load(const char* fn){
  ifstream ifs(fn);
  if (!ifs){
    return FILE_OPEN_ERROR;
  }

  loud_.load(ifs);
  terminal_.load(ifs);
  tail_.load(ifs);

  ifs.read((char*)&keyNum_, sizeof(keyNum_));
  size_t edgesSize = 0;
  ifs.read((char*)&edgesSize, sizeof(edgesSize));
  edges_.resize(edgesSize);
  ifs.read((char*)&edges_[0], sizeof(edges_[0]) * edges_.size());

  size_t tailsNum  = 0;
  ifs.read((char*)&tailsNum,  sizeof(tailsNum));
  vtails_.resize(tailsNum);
  for (size_t i = 0; i < tailsNum; ++i){
    size_t tailSize = 0;
    ifs.read((char*)&tailSize, sizeof(tailSize));
    vtails_[i].resize(tailSize);
    ifs.read((char*)&vtails_[i][0], sizeof(vtails_[i][0]) * vtails_[i].size());
  }


  if (!ifs){
    return FILE_READ_ERROR;
  }
  isReady_ = true;
  return 0;
}

std::string UX::what(const int error){
  switch(error) {
  case 0:
    return string("succeeded");
  case FILE_OPEN_ERROR: 
    return string("file open error");
  case FILE_WRITE_ERROR:
    return string("file write error");
  case FILE_READ_ERROR:
    return string("file read error");
  default:
    return string("unknown error");
  }
}

size_t UX::getAllocSize() const{
  return loud_.getAllocSize() + terminal_.getAllocSize() + 
    tail_.getAllocSize();
}

}
