#pragma once

#include "Runtime/RetroTypes.hpp"

namespace hecl {
class CVarManager;
}

namespace metaforce {

namespace MP1 {

class CTweaks {
public:
  void RegisterTweaks(hecl::CVarManager* cvarMgr);
  void RegisterResourceTweaks(hecl::CVarManager* cvarMgr);
};

} // namespace MP1
} // namespace metaforce
