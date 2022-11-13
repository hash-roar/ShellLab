#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Uitls.h"

namespace shell {
void HandleError(std::string_view message) {
  fprintf(stdout, "error happen: %s\n", message.data());
  std::exit(1);
}

void UnixError(std::string_view message) {
  fprintf(stdout, "%s : %s\n", message.data(), strerror(errno));
  std::exit(1);
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

}  // namespace shell