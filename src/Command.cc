#include "Command.h"

#include <regex.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Utils.h"

const std::unordered_set<std::string> g_special{"&", "<", ">", "|", ">>"};
namespace shell {
bool Command::ParseCmd(const char *cmdline) {
  tokens = split_str(cmdline, " ");
  if (!tokens.empty() && tokens.back() == "\n") {
    tokens.pop_back();
  }
  // TODO: handle too much args
  int argc = 0;
  for (int i = 0; i < tokens.size(); i++) {
    auto &str = tokens[i];
    if (argc == 0) {
      if (g_special.count(str) > 0) {
        printf("parse error near: %s\n", str.c_str());
        return false;
      }
      argv[argc++] = str.data();
      continue;
    }
    if (str == ">") {
      if (i == tokens.size() - 1) {
        printf("> expect file name\n");
        return false;
      }
      oredifile = tokens[i + 1];
      orditype = 0;
      i++;
    } else if (str == ">>") {
      if (i == tokens.size() - 1) {
        printf(">> expect file name\n");
        return false;
      }
      oredifile = tokens[i + 1];
      orditype = 1;
      i++;
    } else if (str == "<") {
      if (i == tokens.size() - 1) {
        printf("< expect file name\n");
        return false;
      }
      iredifile = tokens[i + 1];
      i++;
    } else {
      argv[argc++] = str.data();
    }
  }

  if (argc > 0 && *argv[argc - 1] == '&') {
    argv[argc - 1] = nullptr;
    argc--;
    is_bg = true;
  }

  return true;
}
}  // namespace shell