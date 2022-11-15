#include "Executor.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "Command.h"
#include "Jobs.h"
#include "Utils.h"

#define log(x) \
  printf(x);   \
  fflush(stdout);

constexpr int CMD_BUFFER_SIZE = 1024;
constexpr int MAX_ARG_NUM = 126;
shell::Jobs* g_jobs = nullptr;
volatile std::atomic<pid_t>* g_pid = nullptr;

namespace shell {
void SigchldHandler(int signum) {
  int save_errno = errno;
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    // handle signal interrupt job
    if (WIFSIGNALED(status)) {
      int sig_num = WTERMSIG(status);
      char words[128];
      auto job = g_jobs->GetJobByPid(pid);
      if (!job) return;  // error here
      int n = sprintf(words, "Job [%d] (%d) stopped by signal %d\n", job->jid,
                      job->pid, sig_num);
      write(1, words, n);
    }
    g_jobs->Delete(pid);
    pid_t dummy{pid};
    g_pid->compare_exchange_strong(dummy, 0);
  }

  errno = save_errno;
  return;
}
void SigintHandler(int signum) {
  auto front_job = g_jobs->FrontJob();
  if (!front_job) return;
  pid_t pgid = getpgid(front_job->pid);
  kill(-pgid, SIGINT);
  return;
}

void SigtstpHandler(int signum) {
  auto front_job = g_jobs->FrontJob();
  if (!front_job) return;
  pid_t pgid = getpgid(front_job->pid);
  kill(-pgid, SIGTSTP);
  // handle job
  front_job->state = Job::STOP;
  // print message
  char words[128];
  int n = sprintf(words, "Job [%d] (%d) terminated by signal %d\n",
                  front_job->jid, front_job->pid, signum);
  write(1, words, n);
  pid_t dummy{front_job->pid};
  // not wait foreground job anymore
  g_pid->compare_exchange_strong(dummy, 0);
  return;
}

void SigquitHandler(int signum) {
  printf("Terminating after receipt of SIGQUIT signal\n");
  std::exit(1);
}

void Executor::Run() {
  RegisterSignal(SIGINT, SigintHandler);
  RegisterSignal(SIGTSTP, SigtstpHandler);
  RegisterSignal(SIGCHLD, SigchldHandler);
  RegisterSignal(SIGQUIT, SigquitHandler);
  g_jobs = &jobs_;
  g_pid = &wait_pid_;

  char cmd_buffer[CMD_BUFFER_SIZE] = {};
  while (1) {
    if (emit_prompt_) {
      printf("%s", prompt);
      std::fflush(stdout);
    }
    // get cmd buffer
    if ((std::fgets(cmd_buffer, CMD_BUFFER_SIZE, stdin) == NULL) &&
        std::ferror(stdin)) {
      HandleError("fgets error");
    }
    if (feof(stdin)) {
      std::fflush(stdout);
      std::exit(0);
    }
    // evaluate cmd
    EvalCmd({cmd_buffer});
    std::fflush(stdout);
  }
}

void Executor::EvalCmd(string_view buffer) {
  bool is_bg = false;
  Command cmd{};
  if (!cmd.ParseCmd(buffer.data())) return;

  is_bg = cmd.is_bg;
  char** argv = cmd.argv;

  if (argv[0] == nullptr) return;

  if (RunBuiltinCmd(argv)) {
    return;
  }
  // run binary executable
  pid_t pid;
  // add mask

  // fork
  pid = fork();
  if (pid < 0) UnixError("fork error");
  if (pid == 0) {
    // io redirect
    if (!cmd.iredifile.empty()) {
      int fd = open(cmd.iredifile.c_str(), O_RDONLY);
      if (fd < 0) {
        PrintUnixError("error in open file ");
        return;
      }
      dup2(fd, 0);
    }
    if (!cmd.oredifile.empty()) {
      int flag = cmd.orditype == 1 ? O_TRUNC | O_WRONLY | O_CREAT
                                   : O_APPEND | O_WRONLY | O_CREAT;
      int fd = open(cmd.oredifile.c_str(), flag, S_IRWXU);
      if (fd < 0) {
        PrintUnixError("error in open file ");
        return;
      }
      dup2(fd, 1);
    }
    setpgid(0, 0);
    if (execve(argv[0], argv, environ) < 0) UnixError("execute error");
  }
  // wait
  if (is_bg) {
    jobs_.Add(pid, Job::BACK, string{buffer});
    printf("[%d] (%d) %s", jobs_.Pid2Jid(pid).value(), pid, buffer.data());
  } else {
    fflush(stdout);
    jobs_.Add(pid, Job::FRONT, string{buffer});
    WaitForeground(pid);
  }
}

bool Executor::ParseCmd(char* cmdline, char** argv) {
  char* buf = cmdline; /* ptr that traverses command line */
  char* delim;         /* points to first space delimiter */
  int argc;            /* number of args */
  int bg;              /* background job? */

  buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* ignore leading spaces */
    buf++;

  /* Build the argv list */
  argc = 0;
  if (*buf == '\'') {
    buf++;
    delim = strchr(buf, '\'');
  } else {
    delim = strchr(buf, ' ');
  }

  while (delim) {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) /* ignore spaces */
      buf++;

    if (*buf == '\'') {
      buf++;
      delim = strchr(buf, '\'');
    } else {
      delim = strchr(buf, ' ');
    }
  }
  argv[argc] = nullptr;

  if (argc == 0) /* ignore blank line */
    return 1;

  /* should the job run in the background? */
  if ((bg = (*argv[argc - 1] == '&')) != 0) {
    argv[--argc] = nullptr;
  }
  return bg;
}

bool Executor::RunBuiltinCmd(char** argv) {
  string_view command{argv[0]};
  if (command == "quit") {
    std::exit(1);
  } else if (command == "bg") {
    DoBgCmd(argv);
    return true;
  } else if (command == "fg") {
    DoFgCmd(argv);
    return true;
  } else if (command == "jobs") {
    jobs_.Print();
    return true;
  }
  return false;
}

void Executor::DoBgCmd(char** argv) {
  // validate argument
  if (argv[1] == nullptr) {
    printf("not enough argument");
    return;
  }
  string id(argv[1]);
  pid_t pid;
  int jid;
  bool is_jid;
  if (*id.begin() == '%') {
    is_jid = true;
    jid = std::atoi(id.data() + 1);
    if (jid == 0) {
      printf("invalid jid: %s", argv[1]);
      return;
    }
  } else {
    is_jid = false;
    pid = std::atoi(id.data());
    if (pid == 0) {
      printf("invalid jid: %s", argv[1]);
      return;
    }
  }

  // perform bg action
  auto job = is_jid ? jobs_.GetJobByJid(jid) : jobs_.GetJobByPid(pid);
  if (!job || job->state != Job::STOP) return;  // no target job

  // activate job
  job->state = Job::BACK;
  pid_t pgid = getpgid(job->pid);
  kill(pgid, SIGCONT);
  printf("[%d] (%d) %s", job->jid, job->pid, job->cmd_str.c_str());
}

void Executor::DoFgCmd(char** argv) {
  if (argv[1] == nullptr) {
    printf("not enough argument");
    return;
  }
  string id(argv[1]);
  pid_t pid;
  int jid;
  bool is_jid;
  if (*id.begin() == '%') {
    is_jid = true;
    jid = std::atoi(id.data() + 1);
    if (jid == 0) {
      printf("invalid jid: %s\n", argv[1]);
      return;
    }
  } else {
    is_jid = false;
    pid = std::atoi(id.data());
    if (pid == 0) {
      printf("invalid pid: %s\n", argv[1]);
      return;
    }
  }

  // perform bg action
  auto job = is_jid ? jobs_.GetJobByJid(jid) : jobs_.GetJobByPid(pid);
  if (!job) return;  // no target job

  // activate job
  job->state = Job::FRONT;
  auto pgid = getpgid(job->pid);
  kill(pgid, SIGCONT);
  WaitForeground(job->pid);
}

bool Executor::RunBinaryCmd(bool bg, char** argv) { return false; };

void Executor::WaitForeground(pid_t pid) {
  wait_pid_.store(pid);
  while (wait_pid_.load() != 0) {
    // std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }
}

}  // namespace shell