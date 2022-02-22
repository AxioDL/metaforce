#pragma once

#include "Runtime/Tweaks/ITweakPlayerControl.hpp"

namespace metaforce::MP1 {
struct CTweakPlayerControl final : Tweaks::ITweakPlayerControl {
  std::array<ControlMapper::EFunctionList, 67> m_mappings;
  [[nodiscard]] ControlMapper::EFunctionList GetMapping(u32 command) const override { return m_mappings[command]; }
  CTweakPlayerControl() = default;
  CTweakPlayerControl(CInputStream& reader);
};

} // namespace metaforce::MP1
