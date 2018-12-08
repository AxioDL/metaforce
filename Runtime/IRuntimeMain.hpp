#pragma once

namespace urde {
struct IRuntimeMain {
  void init() = 0;
  int proc() = 0;
  void stop() = 0;
};
} // namespace urde
