#pragma once

#include <string>
namespace shell {
struct Command {
  std::string cmd_buffer{};
  char** argv{nullptr};  // point to cmd_buffer

 public:
  bool ParseCmd(const char* cmdline);
};
}  // namespace shell