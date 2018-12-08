#pragma once

#include <utility>
#include <memory>
#include <string>

namespace urde::MP1 {

class CSaveGameScreenTouchBar {
public:
  virtual ~CSaveGameScreenTouchBar() = default;
  virtual int PopOption();
  virtual void SetUIOpts(std::u16string_view opt0, std::u16string_view opt1, std::u16string_view opt2);
};

std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar();

} // namespace urde::MP1
