#include "Utils.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace shell {
void HandleError(std::string_view message) {
  fprintf(stdout, "error happen: %s\n", message.data());
  std::exit(1);
}

void UnixError(std::string_view message) {
  fprintf(stdout, "%s : %s\n", message.data(), strerror(errno));
  std::exit(1);
}

void PrintUnixError(std::string_view message) {
  fprintf(stdout, "%s : %s\n", message.data(), strerror(errno));
}

SignalHandler* RegisterSignal(int signum, SignalHandler* handler) {
  struct sigaction action, old_action;
  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;

  if (sigaction(signum, &action, &old_action) < 0)
    UnixError("signal action error");
  return old_action.sa_handler;
}

std::vector<std::string> split_str(std::string_view text,
                                   std::string_view delims) {
  std::vector<std::string> tokens;
  std::size_t start = text.find_first_not_of(delims), end = 0;

  while ((end = text.find_first_of(delims, start)) != std::string::npos) {
    tokens.push_back(std::string{text.substr(start, end - start)});
    start = text.find_first_not_of(delims, end);
  }
  if (start != std::string::npos)
    tokens.push_back(std::string{text.substr(start)});
  return tokens;
}

}  // namespace shell