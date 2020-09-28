#pragma once

#include <array>

#include "hsh/hsh.h"

namespace urde {
class CLineRenderer;

class CLineRendererShaders {
public:
  static void BindShader(CLineRenderer& renderer, hsh::texture2d texture, bool additive,
                         hsh::Compare zcomp);
};

} // namespace urde
