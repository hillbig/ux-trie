#include <iostream>
#include <fstream>
#include <string>
#include "cmdline.h"
#include "ux.hpp"

using namespace std;

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

int buildUX(const string& fn, const string& index){
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
  ux.build(wordList);

  cout << "original size:\t" << originalSize << endl
       << "allocate size:\t" << ux.getAllocSize() << " (" << (float)ux.getAllocSize() / originalSize << ")" << endl
       << "     key  num:\t" << wordList.size() << endl;


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
  p.add<string>("wordlist", 'w', "word list", false);
  p.add<string>("index",    'i', "index",     true);
  p.add<int>   ("limit",    'l', "limit at search", false, 10);
  p.add        ("enumerate",'e', "enumerate all keyword");
  p.add("help", 'h', "this message");
  p.set_program_name("ux");

  if (!p.parse(argc, argv) || p.exist("help")){
    cerr << p.usage() << endl;
    return -1;
  }

  if (p.exist("wordlist")){
    return buildUX(p.get<string>("wordlist"), p.get<string>("index"));
  } else if (p.exist("enumerate")){
    return listUX(p.get<string>("index"));
  } else {
    return searchUX(p.get<string>("index"), p.get<int>("limit"));
  }

  return 0; // NOT COME 
}



