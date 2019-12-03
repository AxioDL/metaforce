#pragma once

#include <memory>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Particle/IElement.hpp"

/* Documentation at: https://wiki.axiodl.com/w/Particle_Script#UV_Elements */

namespace urde {
class CToken;

struct SUVElementSet {
  float xMin, yMin, xMax, yMax;
};

class CUVElement : public IElement {
public:
  virtual TLockedToken<CTexture> GetValueTexture(int frame) const = 0;
  virtual void GetValueUV(int frame, SUVElementSet& valOut) const = 0;
  virtual bool HasConstantTexture() const = 0;
  virtual bool HasConstantUV() const = 0;
};

struct CUVEConstant : public CUVElement {
  TLockedToken<CTexture> x4_tex;

public:
  CUVEConstant(TToken<CTexture>&& tex) : x4_tex(std::move(tex)) {}
  TLockedToken<CTexture> GetValueTexture(int frame) const override { return TLockedToken<CTexture>(x4_tex); }
  void GetValueUV(int frame, SUVElementSet& valOut) const override { valOut = {0.f, 0.f, 1.f, 1.f}; }
  bool HasConstantTexture() const override { return true; }
  bool HasConstantUV() const override { return true; }
};

struct CUVEAnimTexture : public CUVElement {
  TLockedToken<CTexture> x4_tex;
  int x10_tileW, x14_tileH, x18_strideW, x1c_strideH;
  int x20_tiles;
  bool x24_loop;
  std::unique_ptr<CIntElement> x28_cycleFrames;
  std::vector<SUVElementSet> x2c_uvElems;

public:
  CUVEAnimTexture(TToken<CTexture>&& tex, std::unique_ptr<CIntElement>&& tileW, std::unique_ptr<CIntElement>&& tileH,
                  std::unique_ptr<CIntElement>&& strideW, std::unique_ptr<CIntElement>&& strideH,
                  std::unique_ptr<CIntElement>&& cycleFrames, bool loop);
  TLockedToken<CTexture> GetValueTexture(int frame) const override { return TLockedToken<CTexture>(x4_tex); }
  void GetValueUV(int frame, SUVElementSet& valOut) const override;
  bool HasConstantTexture() const override { return true; }
  bool HasConstantUV() const override { return false; }
};

} // namespace urde
