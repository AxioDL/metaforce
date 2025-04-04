#include "Runtime/CInfiniteLoopDetector.hpp"
#include "Runtime/Logging.hpp"

namespace metaforce {
namespace {
std::chrono::system_clock::time_point g_WatchDog = std::chrono::system_clock::now();
std::mutex g_mutex;
} // namespace

CInfiniteLoopDetector::CInfiniteLoopDetector(int duration)
: m_duration(duration), m_futureObj(m_stopRequested.get_future()) {}

bool CInfiniteLoopDetector::stopRequested() const {
  return m_futureObj.wait_for(std::chrono::milliseconds(0)) != std::future_status::timeout;
}
void CInfiniteLoopDetector::run() {
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while (!stopRequested()) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start) >
        std::chrono::milliseconds(m_duration)) {
      std::lock_guard<std::mutex> guard(g_mutex);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - g_WatchDog) >
          std::chrono::milliseconds(m_duration)) {
        spdlog::fatal("INFINITE LOOP DETECTED!");
      }
    }
  }
}

void CInfiniteLoopDetector::UpdateWatchDog(std::chrono::system_clock::time_point time) {
  std::lock_guard<std::mutex> guard(g_mutex);
  g_WatchDog = time;
}

void CInfiniteLoopDetector::stop() { m_stopRequested.set_value(); }
} // namespace metaforce
