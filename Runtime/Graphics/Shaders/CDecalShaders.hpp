#pragma once

#include "hsh/hsh.h"

namespace urde {
struct CQuadDecal;

class CDecalShaders {
public:
  static void BuildShaderDataBinding(hsh::binding& binding, CQuadDecal& decal, hsh::texture2d tex);
};

} // namespace urde
