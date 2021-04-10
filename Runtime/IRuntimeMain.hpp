#pragma once

namespace metaforce {
struct IRuntimeMain {
  void init() = 0;
  int proc() = 0;
  void stop() = 0;
};
} // namespace metaforce
