/* 
 *  Copyright (c) 2007-2010 Daisuke Okanohara 
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
#include "ux.hpp"

using namespace std;

namespace ux_tool{

UX::UX() : keyNum_(0), isReady_(false) {
} 

UX::~UX(){
}

struct RangeNode{
  RangeNode(size_t _left, size_t _right, size_t _depth) :
    left(_left), right(_right), depth(_depth) {}
  size_t left;
  size_t right;
  size_t depth;
};

void UX::build(vector<string>& wordList){
  sort(wordList.begin(), wordList.end());
  wordList.erase(unique(wordList.begin(), wordList.end()), wordList.end());

  keyNum_ = wordList.size();

  size_t totalLen = 0;
  for (size_t i = 0; i < wordList.size(); ++i){
    totalLen += wordList[i].size();
  }

  queue<RangeNode> q;
  if (keyNum_ != 0){
    q.push(RangeNode(0, keyNum_, 0));
  }
  
  loud_.push_back(0); // super root
  loud_.push_back(1);


  size_t nodeNum = 0;
  while (!q.empty()){
    RangeNode& rn = q.front();
    const size_t depth = rn.depth;
    const size_t left  = rn.left;
    const size_t right = rn.right;
    q.pop();
    nodeNum++;
    
    size_t newLeft = left;
    if (wordList[left].size() == depth){
      
      terminal_.push_back(1);
      ++newLeft;
      if (newLeft == right) {
	loud_.push_back(1);
	continue;
      }
    } else {
      terminal_.push_back(0);
    }

    size_t prev   = newLeft;
    uint8_t prevC = (uint8_t)wordList[prev][depth];

    for (size_t i = prev+1; ; ++i){
      if (i < right &&
	  prevC == (uint8_t)wordList[i][depth]){
	continue;
      }
      edges_.push_back(prevC);
      loud_.push_back(0);
      q.push(RangeNode(prev, i, depth+1));
      if (i == right) break;
      prev  = i;
      prevC = wordList[prev][depth];
    }
    loud_.push_back(1);
  }

  cout << "    loud:" << loud_.size() << endl
       << "terminal:" << terminal_.size() << endl;
  size_t total = loud_.size() + terminal_.size();
  cout << "   total:" << total << endl; 

  loud_.build();
  terminal_.build();
  isReady_ = true;

}

uint32_t UX::getChild(const uint32_t pos, const uint8_t c) const {
  uint32_t curPos  = pos;
  uint32_t edgePos = loud_.rank(pos, 0) - 2;
  for (;; ++curPos, ++edgePos){
    if (loud_.getBit(curPos)){
      break;
    }
    if (edges_[edgePos] == c){
      return loud_.select(loud_.rank(curPos, 0), 1)+1;
    }
  }
  return NOTFOUND;
}

uint32_t UX::getParent(const uint32_t pos, uint8_t& c) const {
  c = edges_[loud_.rank(pos, 0) - 2];
  return loud_.select(loud_.rank(pos-1, 1), 0);
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
  
  uint32_t curPos = 2;
  for (size_t i = 0; i < len; ++i){
    uint32_t nextPos = getChild(curPos, (uint8_t)str[i]);
    if (nextPos == NOTFOUND){
      return 0;
    }
    curPos = nextPos;
  }

  // search all descendant nodes from curPos
  enumerateAll(curPos, retIDs, limit);
  return retIDs.size();
}

void UX::enumerateAll(const uint32_t pos, vector<id_t>& retIDs, const size_t limit) const{
  const uint32_t nodeID = loud_.rank(pos-1, 1) - 1;
  if (terminal_.getBit(nodeID)){
    retIDs.push_back(terminal_.rank(nodeID, 1) - 1);
  }
  for (uint32_t curPos = pos ; loud_.getBit(curPos) == 0 &&
	 retIDs.size() < limit; ++curPos){
    enumerateAll(loud_.select(loud_.rank(curPos, 0), 1) + 1, retIDs, limit);
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
  if (terminal_.size() <= 2) return; 

  uint32_t curPos = 2;
  for (size_t i = 0; curPos != NOTFOUND; ++i){
    const uint32_t nodeID = loud_.rank(curPos-1, 1)-1;
    if (terminal_.getBit(nodeID)){
      lastLen = i;
      retIDs.push_back(terminal_.rank(nodeID, 1)-1);
      if (retIDs.size() == limit) break;
    }
    if (i == len) break;
    curPos = getChild(curPos, (uint8_t)str[i]);
  }
}

void UX::reverseLookup(const id_t id, string& ret) const{
  ret.clear();
  if (id >= keyNum_) return;
  if (terminal_.size() <= 2) return;
  uint32_t nodeID = terminal_.select(id + 1, 1);
  uint8_t  unusedC = 0;
  uint32_t curPos = getParent(loud_.select(nodeID+1, 1) + 1, unusedC);
  while (curPos >= 2){
    uint8_t c = 0;
    curPos = getParent(curPos, c);
    ret += (char)c;
  } 
  reverse(ret.begin(), ret.end());
}

string UX::reverseLookup(const id_t id) const {
  std::string ret;
  reverseLookup(id, ret);
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

  size_t edgesSize = edges_.size();
  ofs.write((const char*)&edgesSize, sizeof(edgesSize));
  ofs.write((const char*)&edges_[0], sizeof(edges_[0]) * edges_.size()); 
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
  size_t edgesSize = 0;
  ifs.read((char*)&edgesSize, sizeof(edgesSize));
  edges_.resize(edgesSize);

  keyNum_ = terminal_.rank(terminal_.size(), 1);

  ifs.read((char*)&edges_[0], sizeof(edges_[0]) * edges_.size());
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
  return loud_.getAllocSize() + terminal_.getAllocSize(); // + edges_.size();
}

size_t UX::getNodeNum() const {
  return loud_.size() / 2 - 1;
}

}
