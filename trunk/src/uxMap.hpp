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

#ifndef UX_MAP_HPP__
#define UX_MAP_HPP__

#include <vector>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include "ux.hpp"

namespace ux_tool{

/**
 * Succict Map using UX
 */
template <class V>
class UXMap{
public:
  /**
   * Constructor
   */
  UXMap() : size_(0){}


  /**
   * Destructor
   */
  ~UXMap() {}

  /**
   * Build a map without keys
   * @param keys keys to be associated
   */
  void build(std::vector<std::string>& keys){
    ux_.build(keys);
    vs_.resize(ux_.size());
  }
  
  /**
   * Build a map from std::map
   * @param m A std::map as an input
   */
  void build(const std::map<std::string, V>& m){
    std::vector<std::string> wordList;
    for (typename std::map<std::string, V>::const_iterator it = 
	   m.begin(); it != m.end(); ++it){
      wordList.push_back(it->first);
    }
    ux_.build(wordList);
    vs_.resize(wordList.size());
    for (typename std::map<std::string, V>::const_iterator it = 
	   m.begin(); it != m.end(); ++it){
      if (set(it->first, it->second) != 0){
	return;
      }
    }
  }

  /**
   * Build a map from the vector of the pair of a key and a value
   * @param kvs A vector of the pair of a key and vlaue
   */
  void build(const std::vector< std::pair<std::string, V> >& kvs){
    std::vector<std::string> wordList;
    for (size_t i = 0; i < kvs.size(); ++i){
      wordList.push_back(kvs[i].first);
    }

    ux_.build(wordList);
    vs_.resize(wordList.size());

    for (size_t i = 0; i < kvs.size(); ++i){
      assert(set(kvs[i].first, kvs[i].second) == 0);
    }
  }

  /**
   * Get a value for a given key
   * @param key A key to be associated
   * @param v An associated value for a key
   * @return 0 on success and -1 if not found
   */
  int get(const std::string& key, V& v) const {
    size_t retLen = 0;
    id_t id = ux_.prefixSearch(key.c_str(), key.size(), retLen);
    if (id == NOTFOUND){
      return -1;
    } 
    v = vs_[id];
    return 0;
  }

  /**
   * Set a value for a given key
   * @param key A key to be associated
   * @param v  A value to be associated for a key
   * @return 0 on success and -1 if not found
   */
  int set(const std::string& key, const V& v){
    size_t retLen = 0;
    id_t id = ux_.prefixSearch(key.c_str(), key.size(), retLen);
    if (id == NOTFOUND){
      return -1;
    }
    vs_[id] = v;
    return 0;
  }

  /**
   * Return the longest key that matches the prefix of the query in the dictionary
   * @param str the query
   * @param len the length of the query
   * @param retLen The length of the matched key in the dictionary 
   * @return The ID of the matched key or NOTFOUND if no key is matched
   */
  int prefixSearch(const char* str, size_t len, size_t& retLen, V& v) const {
    id_t id = ux_.prefixSearch(str, len, retLen);
    if (id == NOTFOUND){
      return -1;
    }
    return vs_[id];
  }

  /** 
   * Return the all keys that match the prefix of the query in the dictionary
   * @param str the query
   * @param len the length of the query
   * @param retIDs The IDs of the matched keys
   * @param limit The maximum number of matched keys
   * @return The number of matched keys
   */
  size_t commonPrefixSearch(const char* str, size_t len, std::vector<V>& v, size_t limit = LIMIT_DEFAULT) const {
    v.clear();
    std::vector<id_t> retIDs;
    commonPrefixSearch(str, len, retIDs, limit);
    v.resize(retIDs.size());
    for (size_t i = 0; i < retIDs.size(); ++i){
      v[i] = vs_[retIDs[i]];
    }
    return v.size();
  }

  /** 
   * Return the all keys whose their prefixes  match the query 
   * @param str the query
   * @param len the length of the query
   * @param The IDs of the matched keys
   * @param limit The maximum number of matched keys
   * @return The number of matched keys
   */
  size_t predictiveSearch(const char* str, size_t len, std::vector<V>& v, size_t limit = LIMIT_DEFAULT) const {
    v.clear();
    std::vector<id_t> retIDs;
    predictiveSearch(str, len, retIDs, limit);
    v.resize(retIDs.size());
    for (size_t i = 0; i < retIDs.size(); ++i){
      v[i] = vs_[retIDs[i]];
    }
    return v.size();
  }

  /**
   * Return the key for the given ID
   * @param id The ID of the key
   * @param ret The key for the given ID or empty if such ID does not exist
   */ 
  void decodeKey(const size_t ind, std::string& ret) const {
    ux_.decodeKey(ind, ret);
  }

  /**
   * Get the number of keys 
   * @return the number of keys
   */
  size_t size() const {
    return ux_.size();
  }

private:
  UX ux_;
  std::vector<V> vs_;
  size_t size_;
};


}


#endif // UX_MAP_HPP__
