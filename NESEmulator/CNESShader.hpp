#pragma once

#include "hsh/hsh.h"
#include "Graphics/CGraphics.hpp"

namespace urde::MP1 {

class CNESShader {
public:
  static void BuildShaderDataBinding(hsh::binding& binding, hsh::vertex_buffer<TexUVVert> vbo,
                                     hsh::uniform_buffer<ViewBlock> uniBuf, hsh::texture2d tex);
};

} // namespace urde::MP1
