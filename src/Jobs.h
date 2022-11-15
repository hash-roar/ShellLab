#pragma once

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <list>
#include <optional>
#include <string>
#include <string_view>
namespace shell {

constexpr int MAX_JOB_SIZE = 256;

struct Job {
  enum State { FRONT, BACK, STOP };
  pid_t pid;
  int jid;
  State state;
  std::string cmd_str;
  Job(pid_t pid, int jid, State st, std::string str)
      : pid(pid), jid(jid), state(st), cmd_str(std::move(str)) {}
};

class Jobs {
 public:
  Jobs() = default;
  Jobs(int cap) : capacity_(cap) {}
  ~Jobs() = default;
  Job* Add(pid_t pid, Job::State st, std::string_view cmd_str) {
    if (pid < 1) return nullptr;
    jobs_.emplace_front(pid, next_jid_.fetch_add(1), st, std::string{cmd_str});
    return &*jobs_.begin();
  }
  void Delete(pid_t pid) {
    auto itr =
        std::find_if(jobs_.begin(), jobs_.end(),
                     [&](const Job& job) -> bool { return job.pid == pid; });
    if (itr == jobs_.end()) return;
    jobs_.erase(itr);
  }
  pid_t FrontPid() {
    auto job = FrontJob();
    if (job == nullptr) return 0;
    return job->pid;
  }

  Job* FrontJob() {
    auto itr = std::find_if(
        jobs_.begin(), jobs_.end(),
        [&](const Job& job) -> bool { return job.state == Job::FRONT; });
    if (itr == jobs_.end()) {
      return nullptr;
    }

    return &*itr;
  }

  void Print() {
    std::for_each(jobs_.begin(), jobs_.end(), [&](const Job& job) {
      std::printf("[%d] (%d) %s%s", job.jid, job.pid, State2Str(job.state),
                  job.cmd_str.c_str());
    });
  }

  Job* GetJobByPid(pid_t pid) {
    auto itr =
        std::find_if(jobs_.begin(), jobs_.end(),
                     [&](const Job& job) -> bool { return job.pid == pid; });
    if (itr == jobs_.end()) return nullptr;
    return &*itr;
  }
  Job* GetJobByJid(int jid) {
    auto itr =
        std::find_if(jobs_.begin(), jobs_.end(),
                     [&](const Job& job) -> bool { return job.jid == jid; });
    if (itr == jobs_.end()) return nullptr;
    return &*itr;
  }

  std::optional<int> Pid2Jid(pid_t pid) {
    auto job = GetJobByPid(pid);
    return job == nullptr ? std::nullopt : std::make_optional(job->jid);
  }

 private:
  const char* State2Str(Job::State state) {
    switch (state) {
      case Job::FRONT:
        return "Foreground ";
      case Job::BACK:
        return "Running  ";
      case Job::STOP:
        return "Stopped ";
    }
    return "";
  }

 private:
  std::list<Job> jobs_;
  std::atomic_uint32_t next_jid_{0};
  size_t capacity_{MAX_JOB_SIZE};
};

}  // namespace shell