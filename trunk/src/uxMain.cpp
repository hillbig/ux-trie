#include <iostream>
#include <fstream>
#include <string>
#include "cmdline.h"
#include "ux.hpp"

using namespace std;

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void analyzeWordList(const vector<string>& wordList){
  size_t lcs = 0;
  for (size_t i = 1; i < wordList.size(); ++i){
    const string& s1 = wordList[i-1];
    const string& s2 = wordList[i];
    size_t j = 0;
    for (; j < s1.size() && j < s2.size() && s1[j] == s2[j]; ++j) {};
    lcs += j;
  }
  cout << "  avelcs:\t" << (float)lcs / wordList.size() << endl;
}

void printQuery(const ux_tool::UX& ux,
		const std::string& query,
		const int limit){
  cout << "query:[" << query << "]" << endl;
  
  // prefixSearch
  size_t retLen = 0;
  cout << "prefixSearch: ";
  ux_tool::id_t id = ux.prefixSearch(query.c_str(), query.size(), retLen);
  if (id == ux_tool::NOTFOUND){
    cout << "not found." << endl;
  } else {
    cout << ux.decode(id) << "\t(id=" << id << ")" << endl;
  }

  vector<id_t> retIDs;  
  // commonPrefixSearch
  ux.commonPrefixSearch(query.c_str(), query.size(), retIDs, limit);
  cout << "commonPrefixSearch: " << retIDs.size() << " found." << endl;
  for (size_t i = 0; i < retIDs.size(); ++i){
    cout << ux.decode(retIDs[i]) << "\t(id=" << retIDs[i] << ")" << endl;
  }

  // predictiveSearch
  ux.predictiveSearch(query.c_str(), query.size(), retIDs, limit);
  cout << "predictiveSearch: " << retIDs.size() << " found." << endl;
  for (size_t i = 0; i < retIDs.size(); ++i){
    cout << ux.decode(retIDs[i]) << "\t(id=" << retIDs[i] << ")" << endl;
  }
}

int buildUX(const string& fn, const string& index, const bool uncompress){
  ifstream ifs(fn.c_str());
  if (!ifs){
    cerr << "cannot open " << fn << endl;
    return -1;
  }
  vector<string> wordList;
  string word;
  size_t originalSize = 0;
  while (getline(ifs, word)){
    if (word.size() > 0 &&
	word[word.size()-1] == '\r'){
      word = word.substr(0, word.size()-1);
    }
    
    wordList.push_back(word);
    originalSize += word.size();
  }
  ux_tool::UX ux;
  double start = gettimeofday_sec();
  ux.build(wordList, !uncompress);
  double end   = gettimeofday_sec();
  ux.allocStat(ux.getAllocSize(), cout);
  ux.stat(cout);
  analyzeWordList(wordList);
   
  cout << "originalSize:\t" << originalSize << endl
       << "   indexSize:\t" << ux.getAllocSize() << " (" << (float)ux.getAllocSize() / originalSize << ")" << endl
       << "      keyNum:\t" << wordList.size() << endl
       << "  index time:\t" << end - start << endl;

  random_shuffle(wordList.begin(), wordList.end());

  start = gettimeofday_sec();
  size_t dummy = 0;
  for (size_t i = 0; i < wordList.size() && i < 1000; ++i){
    size_t retLen = 0;
    dummy += ux.prefixSearch(wordList[i].c_str(), wordList[i].size(), retLen);
  }
  end   = gettimeofday_sec();
  cout << "  query time:\t" << end - start << endl; 

  if (dummy == 777){
    cerr << "luckey" << endl;
  }
  

  int err = 0;
  if ((err = ux.save(index.c_str())) != 0){
    cerr << ux.what(err) << " " << index << endl;
    return -1;
  }
  return 0;
}


int searchUX(const string& index, const int limit){
  ux_tool::UX ux;
  int err = 0;
  if ((err = ux.load(index.c_str())) != 0){
    cerr << ux.what(err) << " " << index << endl;
    return -1;
  }
  cout << "read:" << ux.getKeyNum() << " keys" << endl;
  
  string query;
  for (;;){
    putchar('>');
    getline(cin, query);
    if (query.size() == 0){
      break;
    }
    printQuery(ux, query, limit);
  }

  return 0;
}

int listUX(const string& index){
  ux_tool::UX ux;
  int err = 0;
  if ((err = ux.load(index.c_str())) != 0){
    cerr << ux.what(err) << " " << index << endl;
    return -1;
  }
  cerr << "read:" << ux.getKeyNum() << " keys" << endl;
  
  for (size_t i = 0; i < ux.getKeyNum(); ++i){
    cout << "[" << ux.decode(i) << "]" <<  endl;
  }
  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("wordlist",   'w', "word list", false);
  p.add<string>("index",      'i', "index",     true);
  p.add<int>   ("limit",      'l', "limit at search", false, 10);
  p.add        ("uncompress", 'u', "tail is uncompressed");
  p.add        ("enumerate",  'e', "enumerate all keyword");
  p.add("help", 'h', "this message");
  p.set_program_name("ux");

  if (!p.parse(argc, argv) || p.exist("help")){
    cerr << p.usage() << endl;
    return -1;
  }

  if (p.exist("wordlist")){
    return buildUX(p.get<string>("wordlist"), p.get<string>("index"), p.exist("uncompress"));
  } else if (p.exist("enumerate")){
    return listUX(p.get<string>("index"));
  } else {
    return searchUX(p.get<string>("index"), p.get<int>("limit"));
  }

  return 0; // NOT COME 
}
