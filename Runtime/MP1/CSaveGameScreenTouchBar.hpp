#pragma once

#include <memory>
#include <string>
#include <utility>

namespace urde::MP1 {

class CSaveGameScreenTouchBar {
public:
  virtual ~CSaveGameScreenTouchBar() = default;
  virtual int PopOption();
  virtual void SetUIOpts(std::u16string_view opt0, std::u16string_view opt1, std::u16string_view opt2);
};

std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar();

} // namespace urde::MP1
