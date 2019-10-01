#pragma once

#include <memory>
#include <utility>

namespace urde {

class CGameOptionsTouchBar {
public:
  enum class EAction { None, Back, Advance, ValueChange };

  virtual ~CGameOptionsTouchBar() = default;
  virtual EAction PopAction();
  virtual void GetSelection(int& left, int& right, int& value);
  virtual void SetSelection(int left, int right, int value);
};

std::unique_ptr<CGameOptionsTouchBar> NewGameOptionsTouchBar();

} // namespace urde
