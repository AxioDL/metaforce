#pragma once

#include "../../DNACommon/Tweaks/ITweakPlayerControl.hpp"

namespace DataSpec::DNAMP1 {

struct CTweakPlayerControl final : ITweakPlayerControl {
  AT_DECL_DNA_YAML
  Vector<atUint32, AT_DNA_COUNT(67)> m_mappings;
};

} // namespace DataSpec::DNAMP1
