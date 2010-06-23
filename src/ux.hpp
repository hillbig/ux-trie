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

#ifndef UX_HPP__
#define UX_HPP__

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include "bitVec.hpp"

namespace ux_tool{

typedef uint32_t id_t;

enum {
  NOTFOUND      = 0xFFFFFFFFU,
  LIMIT_DEFAULT = 0xFFFFFFFFU
};

class UX{
public:

  enum {
    FILE_OPEN_ERROR  = 1,
    FILE_WRITE_ERROR = 2,
    FILE_READ_ERROR  = 3,
  };

public:
  UX();
  ~UX();

  void build(std::vector<std::string>& wordList, const bool isTailUX = true);
  
  int save(const char* indexName) const;
  int load(const char* indexName);

  int save(std::ofstream& ofs) const;
  int load(std::ifstream& ifs);
  
  id_t prefixSearch(const char* str, const size_t len, size_t& retLen) const;

  size_t commonPrefixSearch(const char* str, const size_t len, std::vector<id_t>& retIDs, 
			    const size_t limit = LIMIT_DEFAULT) const;

  size_t predictiveSearch(const char* str, const size_t len, std::vector<id_t>& retIDs, 
			  const size_t limit = LIMIT_DEFAULT) const;
  
  void decode(const id_t id, std::string& ret) const;
  std::string decode(const id_t id) const;
  
  size_t getKeyNum() const;
  static std::string what(const int error);

  size_t getAllocSize() const;
  void allocStat(size_t allocSize) const;

private:
  void stat(std::vector<std::string>& wordList) const;


  bool isLeaf(const uint32_t pos) const;
  void getChild(const uint8_t c, uint32_t& pos, uint32_t& zeros) const;
  void getParent(uint8_t& c, uint32_t& pos, uint32_t& zeros) const;
  void buildTailUX();
  
  void traverse(const char* str, const size_t len, size_t& retLen, std::vector<id_t>& retIDs, 
		const size_t limit) const;

  void enumerateAll(const uint32_t pos, const uint32_t zeros, std::vector<id_t>& retIDs, const size_t limit) const;
  bool tailMatch(const char* str, const size_t len, const size_t depth,
		 const uint32_t tailID, size_t& retLen) const;
  std::string getTail(const uint32_t i) const;

  BitVec loud_;
  BitVec terminal_;
  BitVec tail_;

  std::vector<std::string> vtails_;
  UX* vtailux_;

  std::vector<uint8_t> edges_;
  std::vector<uint32_t> tailIDs_;

  size_t keyNum_;

  bool isReady_;
};

}

#endif // UX_HPP__
