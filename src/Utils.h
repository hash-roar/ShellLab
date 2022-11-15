#pragma once

#include <unistd.h>

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>
namespace shell {
using SignalHandler = void(int);
[[noreturn]] void HandleError(std::string_view message);
[[noreturn]] void UnixError(std::string_view message);
void PrintUnixError(std::string_view message);

SignalHandler *RegisterSignal(int signum, SignalHandler *handler);

// string helper functions

std::vector<std::string> split_str(std::string_view str,
                                   std::string_view delim);

// template <typename Format, typename... Arg>
// void Print(Format format, Arg... arg);

// signal safe print
template <typename... Arg>
void PrintSf(const char *const format, Arg... arg) {
  char buff[1024];
  int n = sprintf(buff, format, arg...);
  write(1, buff, n);
}

}  // namespace shell