#pragma once

#include <spdlog/spdlog.h> // IWYU pragma: export

namespace spdlog {
template <typename... Args>
[[noreturn]] inline void fatal(format_string_t<Args...> fmt, Args&&... args) {
  default_logger_raw()->critical(fmt, std::forward<Args>(args)...);
  default_logger_raw()->flush();
  std::terminate();
}
} // namespace spdlog
