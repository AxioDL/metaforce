#pragma once

#include <array>
#include "hsh/hsh.h"

namespace urde {
class CElementGen;

class CElementGenShaders {
  hsh::binding m_shaderBind;
public:
  enum class EShaderClass { Tex, IndTex, NoTex };

  static EShaderClass GetShaderClass(CElementGen& gen);
  hsh::binding& BuildShaderDataBinding(CElementGen& gen);
};

} // namespace urde
