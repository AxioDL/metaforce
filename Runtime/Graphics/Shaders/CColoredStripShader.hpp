#pragma once

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

namespace urde {

class CColoredStripShader {
public:
  enum class Mode {
    Alpha,
    Additive,
    FullAdditive,
    Subtractive
  };
  struct Vert {
    zeus::CVector3f m_pos;
    zeus::CColor m_color;
    zeus::CVector2f m_uv;
  };

private:
  struct Uniform {
    zeus::CMatrix4f m_matrix;
    zeus::CColor m_color;
  };
  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  CColoredStripShader(size_t maxVerts, Mode mode, hsh::texture2d tex);
  void draw(const zeus::CColor& color, size_t numVerts, const Vert* verts);
};

}
