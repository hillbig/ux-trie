#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "bitVec.hpp"

using namespace std;

TEST(bitvec, trivial_zero){
  BitVec bv;
  for (int i = 0; i < 1000; ++i){
    bv.push_back(0);
  }
  bv.build();
  ASSERT_EQ(1000, bv.size());
  for (size_t i = 0; i < bv.size(); ++i){
    ASSERT_EQ(0  , bv.getBit(i));
    ASSERT_EQ(i+1, bv.rank(i, 0));
    ASSERT_EQ(i  , bv.select(i+1, 0));
  }
}

TEST(bitvec, trivial_one){
  BitVec bv;
  for (int i = 0; i < 1000; ++i){
    bv.push_back(1);
  }
  bv.build();
  ASSERT_EQ(1000, bv.size());
  for (size_t i = 0; i < bv.size(); ++i){
    ASSERT_EQ(1  , bv.getBit(i));
    ASSERT_EQ(i+1, bv.rank(i, 1));
    ASSERT_EQ(i  , bv.select(i+1, 1));
  }
}


TEST(bitvec, random){
  BitVec bv;
  vector<int> B;
  for (int i = 0; i < 100000; ++i){
    int b = rand() % 2;
    bv.push_back(b);
    B.push_back(b);
  }
  
  bv.build();
  ASSERT_EQ(100000, bv.size());
  int sum = 0;
  for (size_t i = 0; i < bv.size(); ++i){
    ASSERT_EQ(B[i]  , bv.getBit(i));
    sum += B[i];
    if (B[i]){
      ASSERT_EQ(sum, bv.rank(i, 1));
      ASSERT_EQ(i,   bv.select(sum, 1));
    } else {
      ASSERT_EQ(i - sum + 1, bv.rank(i, 0));
      ASSERT_EQ(i,           bv.select(i-sum+1, 0));
    }
  }
}

