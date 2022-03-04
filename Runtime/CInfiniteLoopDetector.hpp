#pragma once

#include <chrono>
#include <mutex>
#include <future>

namespace metaforce {
class CInfiniteLoopDetector {
  int m_duration = 0;
  std::mutex m_mutex;
  std::promise<void> m_stopRequested;
  std::future<void> m_futureObj;
  bool stopRequested() const;
public:
  explicit CInfiniteLoopDetector(int duration=1000);
  void run();
  void stop();
  static void UpdateWatchDog(std::chrono::system_clock::time_point WatchDog);
};
}