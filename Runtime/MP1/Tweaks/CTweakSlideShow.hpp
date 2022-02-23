#pragma once

#include "Runtime/Tweaks/ITweakSlideShow.hpp"
#include "zeus/CColor.hpp"

namespace metaforce::MP1 {

struct CTweakSlideShow final : Tweaks::ITweakSlideShow {
  std::string x4_pakName;
  std::string x14_fontAssetName;
  zeus::CColor x24_fontColor;
  zeus::CColor x28_outlineColor;
  float x2c_scanPercentInterval;
  float x30_;
  float x34_;
  float x38_;
  float x3c_;
  zeus::CColor x40_;
  float x44_;
  float x48_;
  float x4c_;
  float x50_;
  float x54_;
  float x58_;

  CTweakSlideShow() = default;
  CTweakSlideShow(CInputStream& in);

  std::string_view GetFont() const override { return x14_fontAssetName; }
  const zeus::CColor& GetFontColor() const override { return x24_fontColor; }
  const zeus::CColor& GetOutlineColor() const override { return x28_outlineColor; }
  float GetScanPercentInterval() const override { return x2c_scanPercentInterval; }
  float GetX54() const override { return x54_; }
};

} // namespace DataSpec::DNAMP1
