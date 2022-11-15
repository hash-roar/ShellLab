#pragma once

#include <sys/types.h>

#include <atomic>
#include <string_view>
#include <vector>

#include "Jobs.h"

using std::string;
using std::string_view;
// using std::vector;

extern char **environ;
constexpr const char prompt[] = "mysh> ";

namespace shell {
class Executor {
 public:
  Executor() = default;
  Executor(bool prompt) : emit_prompt_(prompt){};
  ~Executor() = default;
  // main run loop
  void Run();
  //   void Init();

 private:
  //  evaluate command
  void EvalCmd(std::string_view cmd_str);

  // parse cmd
  // return true if is background job
  bool ParseCmd(char *cmdline, char **argv);

  // run builtin command
  // return false if not builtin command
  bool RunBuiltinCmd(char **argv);

  // run binary application
  // return true always
  bool RunBinaryCmd(bool bg, char **argv);

  // Wait for foreground job
  void WaitForeground(pid_t pid);

  // do bg command
  void DoBgCmd(char **argv);

  // do fg command
  void DoFgCmd(char **argv);

 private:
  Jobs jobs_;
  volatile std::atomic<pid_t> wait_pid_{0};
  bool emit_prompt_{true};
};

// some signal handler
void SigchldHandler(int signum);
void SigintHandler(int signum);
void SigtstpHandler(int signum);
void SigquitHandler(int signum);

}  // namespace shell