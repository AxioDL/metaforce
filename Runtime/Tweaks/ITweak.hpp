#pragma once

// Gonna need these in all the tweaks anyway, so we'll include them here
#include "zeus/zeus.hpp"
#include "Runtime/GCNTypes.hpp"

namespace hecl {
class CVarManager;
}

namespace metaforce {
class CInputStream;
class COutputStream;
class ITweak {
public:
  virtual ~ITweak() = default;
  virtual void initCVars(hecl::CVarManager*) {}
};
} // namespace metaforce
