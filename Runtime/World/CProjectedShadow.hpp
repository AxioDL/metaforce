#pragma once

#include "Graphics/CTexture.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"

namespace urde {
class CStateManager;
class CModelData;
class CProjectedShadow {
  CTexture x0_texture;
  zeus::CAABox x68_ = zeus::CAABox();
  bool x80_;
  bool x81_persistent;
  float x84_ = 1.f;
  zeus::CVector3f x88_ = zeus::skZero3f;
  float x94_zDistanceAdjust = 0.f;
  float x98_ = 1.f;

public:
  CProjectedShadow(u32, u32, bool);

  zeus::CAABox CalculateRenderBounds();
  void Render(const CStateManager& mgr);
  void RenderShadowBuffer(const CStateManager&, const CModelData&, const zeus::CTransform&, s32, const zeus::CVector3f&,
                          float, float);
};
} // namespace urde
