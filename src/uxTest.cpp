#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "ux.hpp"

using namespace std;

TEST(ux, trivial){
  ux_tool::UX ux;
  vector<string> wordList;
  ux.build(wordList);
  string q = "hoge";
  size_t retLen = 0;
  ASSERT_EQ(ux_tool::NOTFOUND, ux.prefixSearch(q.c_str(), q.size(), retLen));
}

TEST(ux, simple){
  vector<string> wordList;
  wordList.push_back("i");
  wordList.push_back("in");
  wordList.push_back("to");
  wordList.push_back("we");
  wordList.push_back("inn");
  wordList.push_back("tea");
  wordList.push_back("ten");
  vector<string> origWordList = wordList;
  ux_tool::UX ux;
  ux.build(wordList);
  for (size_t i = 0; i < origWordList.size(); ++i){
    ASSERT_EQ(origWordList[i], ux.reverseLookup(i));
  }
}

TEST(ux, reverseLookup){
  ux_tool::UX ux;
  vector<string> wordList;
  wordList.push_back("tok");
  wordList.push_back("osak");
  wordList.push_back("okina");
  wordList.push_back("fukush");
  ux.build(wordList);

  cout <<  ux.reverseLookup(0) << endl;
  cout <<  ux.reverseLookup(1) << endl;
  cout <<  ux.reverseLookup(2) << endl;
  cout <<  ux.reverseLookup(3) << endl;

  ASSERT_EQ("tok", ux.reverseLookup(0));  
  ASSERT_EQ("osak", ux.reverseLookup(1));
  ASSERT_EQ("okina", ux.reverseLookup(2));
  ASSERT_EQ("fukush", ux.reverseLookup(3));


}

TEST(ux, prefixSearch){
  ux_tool::UX ux;
  vector<string> wordList;
  wordList.push_back("tea");
  wordList.push_back("top");
  wordList.push_back("bear");
  wordList.push_back("bep");
  wordList.push_back("東京都");
  ux.build(wordList);

  size_t retLen = 0;
  string q1 = "tea";
  ASSERT_NE(ux_tool::NOTFOUND, ux.prefixSearch(q1.c_str(), q1.size(), retLen));
  ASSERT_EQ(retLen, 3);
  string q2 = "hoge";
  ASSERT_EQ(ux_tool::NOTFOUND, ux.prefixSearch(q2.c_str(), q2.size(), retLen));
  string q3 = "te";
  ASSERT_EQ(ux_tool::NOTFOUND, ux.prefixSearch(q3.c_str(), q3.size(), retLen));
  string q4 = "東京都";
  ASSERT_NE(ux_tool::NOTFOUND, ux.prefixSearch(q4.c_str(), q4.size(), retLen));
}

TEST(ux, commonPrefixSearch){
  ux_tool::UX ux;
  vector<string> wordList;
  wordList.push_back("tea");
  wordList.push_back("top");
  wordList.push_back("bear");
  wordList.push_back("bep");
  wordList.push_back("beppu");
  ux.build(wordList);

  vector<id_t> retIDs;
  string q1 = "beppuhaiiyu";
  ASSERT_EQ(2, ux.commonPrefixSearch(q1.c_str(), q1.size(), retIDs));
  ASSERT_EQ("bep", ux.reverseLookup(retIDs[0]));
  ASSERT_EQ("beppu", ux.reverseLookup(retIDs[1]));
}

TEST(ux, predictiveSearch){
  ux_tool::UX ux;
  vector<string> wordList;
  wordList.push_back("tea");
  wordList.push_back("top");
  wordList.push_back("bear");
  wordList.push_back("bep");
  wordList.push_back("beppu");
  ux.build(wordList);

  vector<id_t> retIDs;
  string q1 = "be";
  ASSERT_EQ(3, ux.predictiveSearch(q1.c_str(), q1.size(), retIDs));
  ASSERT_EQ("bear",  ux.reverseLookup(retIDs[0]));
  ASSERT_EQ("bep",   ux.reverseLookup(retIDs[1]));
  ASSERT_EQ("beppu", ux.reverseLookup(retIDs[2]));
}


TEST(ux, save){
  const char* fn = "uxTestSave.ind";
  ux_tool::UX ux;
  string q1 = "tea";
  string q2 = "top";
  string q3 = "bear";
  string q4 = "bep";
  string q5 = "beppu";

  vector<string> wordList;
  wordList.push_back(q1);
  wordList.push_back(q2);
  wordList.push_back(q3);
  wordList.push_back(q4);
  wordList.push_back(q5);
  ux.build(wordList);
  ASSERT_EQ(0, ux.save(fn));

  ux_tool::UX ux2;
  ASSERT_EQ(0, ux2.load(fn));
  ASSERT_EQ(0, remove(fn));

  ASSERT_EQ(ux.getKeyNum(), ux2.getKeyNum());

  size_t retLen = 0;
  ASSERT_NE(ux_tool::NOTFOUND, ux2.prefixSearch(q1.c_str(), q1.size(), retLen));
  ASSERT_NE(ux_tool::NOTFOUND, ux2.prefixSearch(q2.c_str(), q2.size(), retLen));
  ASSERT_NE(ux_tool::NOTFOUND, ux2.prefixSearch(q3.c_str(), q3.size(), retLen));
  ASSERT_NE(ux_tool::NOTFOUND, ux2.prefixSearch(q4.c_str(), q4.size(), retLen));
  ASSERT_NE(ux_tool::NOTFOUND, ux2.prefixSearch(q5.c_str(), q5.size(), retLen));
}

TEST(ux, large){
  ux_tool::UX ux;
  vector<string> wordList;
  for (int i = 0; i < 10000; ++i){
    ostringstream os;
    os << i;
    wordList.push_back(os.str());
  }
  
  ux.build(wordList);
  map<int, int> dic;
  for (size_t i = 0; i < wordList.size(); ++i){
    size_t retLen = 0;
    dic[ux.prefixSearch(wordList[i].c_str(), wordList[i].size(), retLen)]++;
  }
  ASSERT_EQ(dic.size(), ux.getKeyNum());
}



