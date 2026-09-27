#include "MetroidPrime/Player/CFaceplateDecoration.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"

CFaceplateDecoration::CFaceplateDecoration(const CStateManager& stateMgr) : mId(kInvalidAssetId) {}

void CFaceplateDecoration::Update(float dt, const CStateManager& mgr) {
  CAssetId txtrId = mgr.GetPlayer()->GetVisorSteam().GetTextureId();
  if (txtrId == kInvalidAssetId && mTex.valid()) {
    mTex->Unlock();
    mId = txtrId;
  }

  if (txtrId != mId && txtrId != kInvalidAssetId) {
    mId = txtrId;
    mTex = gpSimplePool->GetObj(SObjectTag('TXTR', mId));
    if (mTex.valid()) {
      mTex->Lock();
    }
  }
}

void CFaceplateDecoration::Draw(const CStateManager& stateMgr) const {
  if (mTex.valid() && mTex->IsLoaded()) {
    CTexture* texture = TToken< CTexture >(*mTex).GetT();
    float alpha = stateMgr.GetPlayer()->GetVisorSteamAlpha();
    if (!close_enough(alpha, 0.f)) {
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                    CCameraFilterPass::kFS_FullscreenQuarters,
                                    CColor::White().WithAlphaOf(alpha), texture, 1.f);
    }
  }
}
