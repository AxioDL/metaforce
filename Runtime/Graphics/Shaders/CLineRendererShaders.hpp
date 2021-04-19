#pragma once

#include <array>

#include "hsh/hsh.h"

namespace metaforce {
class CLineRenderer;

class CLineRendererShaders {
public:
  static void BindShader(CLineRenderer& renderer, hsh::texture2d texture, bool additive,
                         hsh::Compare zcomp);
};

} // namespace metaforce
