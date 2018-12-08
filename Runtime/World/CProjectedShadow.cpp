#include "World/CProjectedShadow.hpp"

namespace urde {

CProjectedShadow::CProjectedShadow(u32 w, u32 h, bool persistent)
: x0_texture(CTexture(ETexelFormat::I4, w, h, 1)), x81_persistent(persistent) {}

zeus::CAABox CProjectedShadow::CalculateRenderBounds() { return {}; }

void CProjectedShadow::Render(const CStateManager& mgr) {}

void CProjectedShadow::RenderShadowBuffer(const CStateManager&, const CModelData&, const zeus::CTransform&, s32,
                                          const zeus::CVector3f&, float, float) {}
} // namespace urde
