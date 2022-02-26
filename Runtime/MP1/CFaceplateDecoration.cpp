#include "Runtime/MP1/CFaceplateDecoration.hpp"

#include "Runtime/Factory/CSimplePool.hpp"
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
      if (m_texFilter)
        m_texFilter = std::nullopt;
    }
  }

  if (x0_id != txtrId && txtrId.IsValid()) {
    if (m_texFilter)
      m_texFilter = std::nullopt;
    x0_id = txtrId;
    x4_tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});
    xc_ready = true;
    x4_tex.Lock();
  }

  if (!m_texFilter && x4_tex.IsLoaded())
    m_texFilter.emplace(EFilterType::Blend, x4_tex);
}

void CFaceplateDecoration::Draw(CStateManager& stateMgr) {
  if (xc_ready && m_texFilter) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CFaceplateDecoration::Draw", zeus::skPurple);
    zeus::CColor color = zeus::skWhite;
    color.a() = stateMgr.GetPlayer().GetVisorSteam().GetAlpha();
    m_texFilter->DrawFilter(EFilterShape::FullscreenQuarters, color, 1.f);
  }
}

} // namespace metaforce::MP1
