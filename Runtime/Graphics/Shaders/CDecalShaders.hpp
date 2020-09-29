#pragma once

#include "hsh/hsh.h"

namespace urde {
struct CQuadDecal;

class CDecalShaders {
  hsh::binding m_dataBind;

public:
  hsh::binding& BuildShaderDataBinding(CQuadDecal& decal, hsh::texture2d tex);
};

} // namespace urde
