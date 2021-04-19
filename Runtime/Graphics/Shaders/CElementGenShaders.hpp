#pragma once

#include <array>
#include "hsh/hsh.h"

namespace metaforce {
class CElementGen;

class CElementGenShaders {
public:
  enum class EShaderClass { Tex, IndTex, NoTex };
  static EShaderClass GetShaderClass(CElementGen& gen);
  static void BuildShaderDataBinding(CElementGen& gen);
};

} // namespace metaforce
