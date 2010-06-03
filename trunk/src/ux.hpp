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

  void build(std::vector<std::string>& wordList);
  
  int save(const char* indexName) const;
  int load(const char* indexName);
  
  id_t prefixSearch(const char* str, const size_t len, size_t& retLen) const;

  size_t commonPrefixSearch(const char* str, const size_t len, std::vector<id_t>& retIDs, 
			    const size_t limit = LIMIT_DEFAULT) const;

  size_t predictiveSearch(const char* str, const size_t len, std::vector<id_t>& retIDs, 
			  const size_t limit = LIMIT_DEFAULT) const;
  
  void reverseLookup(const id_t id, std::string& ret) const;
  std::string reverseLookup(const id_t id) const;
  
  size_t getKeyNum() const;
  static std::string what(const int error);

  size_t getAllocSize() const;
  size_t getNodeNum() const;

private:
  uint32_t getChild(const uint32_t pos, const uint8_t c) const;
  uint32_t getParent(const uint32_t pos, uint8_t& c) const;

  void traverse(const char* str, const size_t len, size_t& retLen, std::vector<id_t>& retIDs, 
		const size_t limit) const;

  void enumerateAll(const uint32_t pos, std::vector<id_t>& retIDs, const size_t limit) const;

  BitVec loud_;
  BitVec terminal_;

  std::vector<uint8_t> edges_;

  size_t keyNum_;
  bool isReady_;
};

}

#endif // UX_HPP__
