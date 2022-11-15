#pragma once

#include <string>
#include <vector>
namespace shell {

struct Command {
  std::string cmd_buffer{};
  std::vector<std::string> tokens{};
  char* argv[256] = {nullptr};  // point to cmd_buffer
  std::string iredifile{};      // input redirect file
  int orditype{-1};             // ouput type  0 >  1 >>
  std::string oredifile{};      // output redirect file
  bool is_bg{false};

 public:
  bool ParseCmd(const char* cmdline);
};
}  // namespace shell