#include "Runtime/MP1/Tweaks/CTweakPlayerControl.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakPlayerControl::CTweakPlayerControl(CInputStream& in) {
  for (u32 i = 0; i < m_mappings.size(); ++i) {
    m_mappings[i] = ControlMapper::EFunctionList(in.ReadLong());
  }
}
}