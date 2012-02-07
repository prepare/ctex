
#include <Util/UniquePtr.h>
#include "gtest/gtest.h"

using namespace tex;

TEST(UniquePtrTest, NonNull) {
  UniquePtr<int> unique;
  EXPECT_FALSE(unique);
  EXPECT_TRUE(!unique);
  int *i = new int[1];
  unique.reset(i);
  EXPECT_TRUE(unique);
  EXPECT_FALSE(!unique);
}

class UniquePtrTestDummy {
public:
  UniquePtrTestDummy(bool *f) : flag(f) {}
  ~UniquePtrTestDummy() {*flag = true;}
  bool *flag;
};

TEST(UniquePtrTest, Reset) {
  UniquePtr<UniquePtrTestDummy> unique;
  UniquePtrTestDummy *nullDummy = NULL;
  EXPECT_FALSE(unique);
  EXPECT_EQ(unique.get(), nullDummy);
  bool destructorFlag = false, flag2;
  UniquePtrTestDummy *dummy = new UniquePtrTestDummy(&destructorFlag);
  unique.reset(dummy);
  EXPECT_TRUE(unique);
  EXPECT_EQ(unique.get(), dummy);
  ASSERT_FALSE(destructorFlag);
  UniquePtrTestDummy *dummy2 = new UniquePtrTestDummy(&flag2);
  unique.reset(dummy2);
  ASSERT_TRUE(unique);
  ASSERT_EQ(unique.get(), dummy2);
  ASSERT_TRUE(destructorFlag);
}

TEST(UniquePtrTest, Take) {
  UniquePtr<int> unique;
  ASSERT_FALSE(unique);
  int *i = new int[1];
  unique.reset(i);
  ASSERT_TRUE(unique);
  int *i2 = unique.take();
  ASSERT_FALSE(unique);
  ASSERT_EQ(i, i2);
}

TEST(UniquePtrTest, Dereference) {
  UniquePtr<int> unique;
  int *i = new int[1], *i2 = new int[1];
  *i = 5;
  *i2 = 6;
  unique.reset(i);
  ASSERT_EQ(*unique, 5);
  unique.reset(i2);
  ASSERT_EQ(*unique, 6);
}

TEST(UniquePtrTest, Member) {
  bool flag = false;
  UniquePtrTestDummy *dummy = new UniquePtrTestDummy(&flag);
  UniquePtr<UniquePtrTestDummy> unique(dummy);
  ASSERT_EQ(dummy->flag, &flag);
  ASSERT_EQ(*(dummy->flag), flag);
  flag = true;
  ASSERT_EQ(*(dummy->flag), flag);
}