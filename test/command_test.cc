#include "Command.h"

#include <gtest/gtest.h>

#include <iostream>
TEST(COMMAND_TEST, TEST_PARSE_CMD) {
  shell::Command cmd{};
  // basic tset
  ASSERT_TRUE(cmd.ParseCmd(" cmd argv1 argv3 \n"));
  ASSERT_STRCASEEQ(cmd.argv[0], "cmd");
  ASSERT_STRCASEEQ(cmd.argv[2], "argv3");

  // io redirect parse
  shell::Command io_redirect{};
  ASSERT_TRUE(io_redirect.ParseCmd("  cat < 1 >> 2 > 3 \n"));
  ASSERT_STRCASEEQ(io_redirect.argv[0], "cat");
  ASSERT_STRCASEEQ(io_redirect.iredifile.c_str(), "1");
  ASSERT_STRCASEEQ(io_redirect.oredifile.c_str(), "3");

  // error
  shell::Command error;
  ASSERT_TRUE(error.ParseCmd(" cat < fas"));
  shell::Command error1;
  ASSERT_FALSE(error1.ParseCmd(" cat < fas >> "));
  shell::Command error2;
  ASSERT_FALSE(error2.ParseCmd(" < > < fas >> "));
}