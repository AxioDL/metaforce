#include "Runtime/MP1/CFaceplateDecoration.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace metaforce::MP1 {

CFaceplateDecoration::CFaceplateDecoration(CStateManager& stateMgr) {}

void CFaceplateDecoration::Update(float dt, CStateManager& stateMgr) {
  CAssetId txtrId = stateMgr.GetPlayer().GetVisorSteam().GetTextureId();
  if (!txtrId.IsValid()) {
    if (xc_ready) {
      x4_tex.Unlock();
      x0_id = txtrId;
    }
  }

  if (x0_id != txtrId && txtrId.IsValid()) {
    x0_id = txtrId;
    x4_tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});
    xc_ready = true;
    x4_tex.Lock();
  }
}

void CFaceplateDecoration::Draw(CStateManager& stateMgr) {
  if (xc_ready && x4_tex) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CFaceplateDecoration::Draw", zeus::skPurple);
    float alpha = stateMgr.GetPlayer().GetVisorSteam().GetAlpha();
    if (!zeus::close_enough(alpha, 0.f)) {
      zeus::CColor color = zeus::skWhite;
      color.a() = alpha;
      CCameraFilterPass::DrawFilter(EFilterType::Blend, EFilterShape::FullscreenQuarters, color, x4_tex.GetObj(), 1.f);
    }
  }
}

} // namespace metaforce::MP1
