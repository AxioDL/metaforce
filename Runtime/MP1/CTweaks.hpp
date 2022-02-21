#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CVarManager;

namespace MP1 {

class CTweaks {
public:
  void RegisterTweaks(CVarManager* cvarMgr);
  void RegisterResourceTweaks(CVarManager* cvarMgr);
};

} // namespace MP1
} // namespace metaforce
