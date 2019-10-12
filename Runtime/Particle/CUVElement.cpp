#include "CUVElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#UV_Elements */

namespace urde {

CUVEAnimTexture::CUVEAnimTexture(TToken<CTexture>&& tex, std::unique_ptr<CIntElement>&& tileW,
                                 std::unique_ptr<CIntElement>&& tileH, std::unique_ptr<CIntElement>&& strideW,
                                 std::unique_ptr<CIntElement>&& strideH, std::unique_ptr<CIntElement>&& cycleFrames,
                                 bool loop)
: x4_tex(std::move(tex)), x24_loop(loop), x28_cycleFrames(std::move(cycleFrames)) {
  tileW->GetValue(0, x10_tileW);
  tileH->GetValue(0, x14_tileH);
  strideW->GetValue(0, x18_strideW);
  strideH->GetValue(0, x1c_strideH);

  const int width = int(x4_tex.GetObj()->GetWidth());
  const int height = int(x4_tex.GetObj()->GetHeight());
  const float widthF = float(width);
  const float heightF = float(height);
  const int xTiles = std::max(1, width / x18_strideW);
  const int yTiles = std::max(1, height / x1c_strideH);

  x20_tiles = xTiles * yTiles;
  x2c_uvElems.reserve(x20_tiles);

  for (int y = yTiles - 1; y >= 0; --y) {
    for (int x = 0; x < xTiles; ++x) {
      const int px = x18_strideW * x;
      const int px2 = px + x10_tileW;
      const int py = x1c_strideH * y;
      const int py2 = py + x14_tileH;

      x2c_uvElems.push_back({
          float(px) / widthF,
          float(py) / heightF,
          float(px2) / widthF,
          float(py2) / heightF,
      });
    }
  }
}

void CUVEAnimTexture::GetValueUV(int frame, SUVElementSet& valOut) const {
  int cv = 1;
  x28_cycleFrames->GetValue(frame, cv);
  float cvf = float(cv) / float(x20_tiles);
  cvf = float(frame) / cvf;

  int tile = int(cvf);
  if (x24_loop) {
    if (cvf >= float(x20_tiles)) {
      tile = int(cvf) % x20_tiles;
    }
  } else {
    if (cvf >= float(x20_tiles)) {
      tile = x20_tiles - 1;
    }
  }

  valOut = x2c_uvElems[tile];
}

} // namespace urde
