#pragma once

// Gonna need these in all the tweaks anyway, so we'll include them here
#include "Runtime/GCNTypes.hpp"

#include "zeus/zeus.hpp"

namespace metaforce {
class CVar;
class CVarManager;
class CInputStream;
class COutputStream;
class ITweak {
public:
  virtual ~ITweak() = default;
  virtual void initCVars(CVarManager*) {}
};
} // namespace metaforce
