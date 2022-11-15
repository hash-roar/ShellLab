#pragma once

#include <string>
#include <string_view>
#include <vector>
namespace shell {
using SignalHandler = void(int);
[[noreturn]] void HandleError(std::string_view message);
[[noreturn]] void UnixError(std::string_view message);

SignalHandler *RegisterSignal(int signum, SignalHandler *handler);

// string helper functions

std::vector<std::string> split_str(std::string_view str,std::string_view delim);

}  // namespace shell