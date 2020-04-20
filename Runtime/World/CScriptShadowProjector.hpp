#pragma once

#include <memory>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CProjectedShadow;

class CScriptShadowProjector : public CActor {
  float xe8_scale;
  zeus::CVector3f xec_offset;
  float xf8_zOffsetAdjust;
  float xfc_opacity;
  float x100_opacityRecip;
  TUniqueId x104_target;
  std::unique_ptr<CProjectedShadow> x108_projectedShadow;
  u32 x10c_textureSize;
  bool x110_24_persistent : 1;
  bool x110_25_shadowInvalidated : 1 = false;

public:
  CScriptShadowProjector(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool,
                         const zeus::CVector3f&, bool, float, float, float, float, s32);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void CreateProjectedShadow();
};
} // namespace urde
