#pragma once

#include <string>
#include <memory>

#include "RetroTypes.hpp"

namespace urde {
class CArchitectureMessage;
class CArchitectureQueue;

class CIOWin {
  std::string x4_name;
  size_t m_nameHash;

public:
  enum class EMessageReturn { Normal = 0, Exit = 1, RemoveIOWinAndExit = 2, RemoveIOWin = 3 };
  virtual ~CIOWin() {}
  CIOWin(std::string_view name) : x4_name(name) { m_nameHash = std::hash<std::string_view>()(name); }
  virtual EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) = 0;
  virtual bool GetIsContinueDraw() const { return true; }
  virtual void Draw() const {}
  virtual void PreDraw() const {}
  std::string_view GetName() const { return x4_name; }
  size_t GetNameHash() const { return m_nameHash; }
};

} // namespace urde
