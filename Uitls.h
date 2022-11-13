#pragma once

#include <string_view>
namespace shell {
using SignalHandler = void(int);
[[noreturn]] void HandleError(std::string_view message);
[[noreturn]] void UnixError(std::string_view message);

SignalHandler *RegisterSignal(int signum, SignalHandler *handler);

}  // namespace shell