#pragma once

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstddef>
#include <list>
#include <optional>
#include <string>
#include <string_view>
namespace shell {

constexpr int MAX_JOB_SIZE = 256;

struct Job {
  enum State { DEAD, FRONT, BACK, STOP };
  pid_t pid;
  int jid;
  State state;
  std::string cmd_str;
};

class Jobs {
 public:
  Jobs() = default;
  Jobs(int cap) : capacity_(cap) {}
  ~Jobs() = default;
  Job* Add(pid_t pid, Job::State st, std::string_view cmd_str) {
    if (pid < 1) return nullptr;
    jobs_.emplace_front(pid, next_jid_.fetch_add(1), st, cmd_str);
    return &*jobs_.begin();
  }
  void Delete(pid_t pid) {
    auto itr = std::find_if(jobs_.begin(), jobs_.end(),
                            [&](Job& job) -> bool { return job.pid == pid; });
    if (itr == jobs_.end()) return;
    jobs_.erase(itr);
  }
  pid_t FrontPid() const {
    auto itr = std::find_if(jobs_.begin(), jobs_.end(), [&](Job& job) -> bool {
      return job.pid == Job::FRONT;
    });
    if (itr == jobs_.end()) return 0;
    return itr->pid;
  }

  Job* GetJobByPid(pid_t pid) {
    auto itr = std::find_if(jobs_.begin(), jobs_.end(),
                            [&](Job& job) -> bool { return job.pid == pid; });
    if (itr == jobs_.end()) return nullptr;
    return &*itr;
  }
  Job* GetJobByJid(int jid) {
    auto itr = std::find_if(jobs_.begin(), jobs_.end(),
                            [&](Job& job) -> bool { return job.jid == jid; });
    if (itr == jobs_.end()) return nullptr;
    return &*itr;
  }

  std::optional<int> Pid2Jid(pid_t pid) {
    auto job = GetJobByPid(pid);
    return job == nullptr ? std::nullopt : std::make_optional(job->jid);
  }

 private:
  std::list<Job> jobs_;
  std::atomic_uint32_t next_jid_{0};
  size_t capacity_{MAX_JOB_SIZE};
};

}  // namespace shell