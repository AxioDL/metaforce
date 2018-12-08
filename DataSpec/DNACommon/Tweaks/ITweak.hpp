#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace hecl {
class CVarManager;
}
namespace DataSpec {
struct ITweak : BigDNA {

  virtual void initCVars(hecl::CVarManager*) {}
};
} // namespace DataSpec
