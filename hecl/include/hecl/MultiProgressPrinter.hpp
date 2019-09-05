#pragma once

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include "hecl/SystemChar.hpp"

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace hecl {

class MultiProgressPrinter {
  std::thread m_logThread;
  mutable std::mutex m_logLock;
  bool m_newLineAfter;

  struct TermInfo {
#if _WIN32
    HANDLE console;
#endif
    int width;
    bool xtermColor = false;
    bool truncate = false;
  } m_termInfo;

  struct ThreadStat {
    hecl::SystemString m_message, m_submessage;
    float m_factor = 0.f;
    bool m_active = false;
    void print(const TermInfo& tinfo) const;
  };
  mutable std::vector<ThreadStat> m_threadStats;

  mutable float m_mainFactor = -1.f;
  mutable int m_indeterminateCounter = 0;
  mutable int m_curThreadLines = 0;
  mutable int m_curProgLines = 0;
  mutable int m_latestThread = -1;
  mutable bool m_running = false;
  mutable bool m_dirty = false;
  mutable bool m_mainIndeterminate = false;
  uint64_t m_lastLogCounter = 0;
  void LogProc();
  void DoPrint();
  void DrawIndeterminateBar();
  void MoveCursorUp(int n);

public:
  MultiProgressPrinter(bool activate = false);
  ~MultiProgressPrinter();
  void print(const hecl::SystemChar* message, const hecl::SystemChar* submessage, float factor = -1.f,
             int threadIdx = 0) const;
  void setMainFactor(float factor) const;
  void setMainIndeterminate(bool indeterminate) const;
  void startNewLine() const;
  void flush() const;
};

} // namespace hecl
