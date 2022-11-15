#include "Utils.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace shell;

TEST(UTIL_TEST, DISABLED_TEST_SPLIT) {
  // test basic
  auto basic = split_str("cm arg1 arg2", " ");
  for (auto &str : basic) {
    std::cout << str << "\n";
  }
  ASSERT_STRCASEEQ(basic[0].c_str(), "cm");
  ASSERT_STRCASEEQ(basic[2].c_str(), "arg2");

  // test white space
  auto white = split_str("  cm arg1 argv2   v ", " ");
  for (auto &str : white) {
    std::cout << str << "\n";
  }
  ASSERT_STRCASEEQ(white[0].c_str(), "cm");
  ASSERT_STRCASEEQ(white[2].c_str(), "argv2");
  ASSERT_STRCASEEQ(white[3].c_str(), "v");
  // test empty
  auto empty = split_str("", "f");
  ASSERT_TRUE(empty.empty());
  empty = split_str(" a fd dfa", "f");
  for (auto &str : empty) {
    std::cout << str << "\n";
  }
  // test corner
  auto corner = split_str("ss fa ss dfdssdfd \n\n", "ss");
  for (auto &str : corner) {
    std::cout << str << "\n";
  }
}

TEST(PRINT_TEST, TEST_PRINTSF) { PrintSf("hello: %s\n", "word"); }