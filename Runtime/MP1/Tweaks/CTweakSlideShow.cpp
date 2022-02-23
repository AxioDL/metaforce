#include "Runtime/MP1/Tweaks/CTweakSlideShow.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakSlideShow::CTweakSlideShow(CInputStream& in) {
  x4_pakName = in.Get<std::string>();
  x14_fontAssetName = in.Get<std::string>();
  x24_fontColor = in.Get<zeus::CColor>();
  x28_outlineColor = in.Get<zeus::CColor>();
  x2c_scanPercentInterval = in.ReadFloat();
  x30_ = in.ReadFloat();
  x34_ = in.ReadFloat();
  x38_ = in.ReadFloat();
  x3c_ = in.ReadFloat();
  x40_ = in.Get<zeus::CColor>();
  x44_ = in.ReadFloat();
  x48_ = in.ReadFloat();
  x4c_ = in.ReadFloat();
  x50_ = in.ReadFloat();
  x54_ = in.ReadFloat();
  x58_ = in.ReadFloat();
}
} // namespace metaforce::MP1