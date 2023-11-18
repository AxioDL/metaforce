#pragma once

#include "Runtime/Tweaks/ITweak.hpp"

namespace metaforce::Tweaks {

struct ITweakSlideShow : ITweak {
  virtual std::string_view GetFont() const = 0;
  virtual const zeus::CColor& GetFontColor() const = 0;
  virtual const zeus::CColor& GetOutlineColor() const = 0;
  virtual float GetScanPercentInterval() const = 0;
  virtual float GetX54() const = 0;
};

} // namespace DataSpec
