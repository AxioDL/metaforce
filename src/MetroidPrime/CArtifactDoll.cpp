#include "MetroidPrime/CArtifactDoll.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/math.hpp"
#include "rstl/allocator.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

const CColor CArtifactDoll::skPreColor((uchar)255, 160, 5, 255);
const CColor CArtifactDoll::skPostColor((uchar)103, 174, 225, 205);

namespace {
static const char* const ArtifactPieceModels[] = {
    "CMDL_Piece1",  // Truth
    "CMDL_Piece2",  // Strength
    "CMDL_Piece3",  // Elder
    "CMDL_Piece4",  // Wild
    "CMDL_Piece5",  // Lifegiver
    "CMDL_Piece6",  // Warrior
    "CMDL_Piece7",  // Chozo
    "CMDL_Piece8",  // Nature
    "CMDL_Piece9",  // Sun
    "CMDL_Piece10", // World
    "CMDL_Piece11", // Spirit
    "CMDL_Piece12"  // Newborn
};
static const CAssetId ArtifactScanIds[] = {
    0x32C9DDCEu, // Truth
    0xB45DAF60u, // Strength
    0x7F017CC5u, // Elder
    0x62044C7Du, // Wild
    0xA9589FD8u, // Lifegiver
    0x2FCCED76u, // Warrior
    0xE4903ED3u, // Chozo
    0x15C68C06u, // Nature
    0xDE9A5FA3u, // Sun
    0xFBBE9D9Au, // World
    0x30E24E3Fu, // Spirit
    0xB6763C91u  // Newborn
};
}; // namespace

CArtifactDoll::CArtifactDoll()
: mLights(2, CLight::BuildDirectional(CVector3f::Forward(), CColor(0xFFFFFFFF)),
          rstl::rmemory_allocator())
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, mFader(0.f)
, mIsLoaded(0) {

  mModels.reserve(12);
  for (int i = 0; i < ARRAY_SIZE(ArtifactPieceModels); ++i) {
    CToken model = gpSimplePool->GetObj(ArtifactPieceModels[i]);
    model.Lock();
    mModels.push_back(model);
  }
}

bool CArtifactDoll::CheckLoadComplete() {
  if (IsLoaded()) {
    return true;
  }

  for (rstl::vector< CToken >::iterator it = mModels.begin(); it != mModels.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  mIsLoaded = true;

  return true;
}

const bool CArtifactDoll::IsLoaded() const { return !!mIsLoaded; }

CArtifactDoll::~CArtifactDoll() {}

void CArtifactDoll::UpdateActorLights() {
  // Retail computes this direction but uses the forward vector for the first light.
  (CVector3f::Forward() + (CVector3f::Right() * 0.25f) + (CVector3f::Down() * 0.1f))
      .AsNormalized();
  mLights[0] = CLight::BuildDirectional(CVector3f::Forward(), CColor((uchar)255, 255, 255));
  mLights[1] = CLight::BuildDirectional(-CVector3f::Forward(), CColor((uchar)0, 0, 0));
  mActorLights->BuildFakeLightList(mLights, CColor(0.25f, 0.25f, 0.25f));
}

void CArtifactDoll::Touch() {
  if (!CheckLoadComplete()) {
    return;
  }

  for (int i = 0; i < mModels.size(); ++i) {
    TToken< CModel >(mModels[i])->Touch(0);
  }
}

void CArtifactDoll::Update(float dt, const CStateManager& mgr) {
  if (!CheckLoadComplete()) {
    return;
  }

  mFader = rstl::min_val(1.f, mFader + 2.f * dt);

  if (close_enough(mFader, 1.f)) {
    UpdateArtifactHeadScan(mgr, 0.5f * dt / 2.f);
  }
  UpdateActorLights();
}

void CArtifactDoll::CompleteArtifactHeadScan(const CStateManager& mgr) {
  UpdateArtifactHeadScan(mgr, 1.f);
}

void CArtifactDoll::UpdateArtifactHeadScan(const CStateManager& mgr, float delta) {
  for (int i = 0; i < ARRAY_SIZE(ArtifactScanIds); ++i) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType(i + CPlayerState::kIT_Truth))) {
      CAssetId id = ArtifactScanIds[i];
      const float currentTime = mgr.GetPlayerState()->GetScanTime(id);
      const float scanTime = currentTime + delta;
      mgr.GetPlayerState()->SetScanTime(id, rstl::min_val(1.f, scanTime));
    }
  }
}

void CArtifactDoll::Draw(float alpha, const CStateManager& mgr, const bool inArtifactCategory,
                         const int selectedArtifact) {
  if (!IsLoaded()) {
    return;
  }

  const float effectiveAlpha = alpha * mFader;
  gpRender->SetPerspective(55.f, CGraphics::GetViewportWidth(), CGraphics::GetViewportHeight(),
                          0.2f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetViewPointMatrix(CTransform4f::Translate(0.f, -10.f, 0.f));
  mActorLights->ActivateLights();
  const float angle = CGraphics::GetSecondsMod900() * M_PIF * 2.f * 0.25f;
  const float xAngle = 8.f * CMath::SlowSineR(angle);
  float zAngle = 8.f * CMath::SlowCosineR(angle);
  CGraphics::SetModelMatrix(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) *
                            CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) *
                            CTransform4f::RotateX(CRelAngle::FromRadians(M_PIF / 2.f)) * CTransform4f::Scale(0.2f));

  const CPlayerState& playerState = *mgr.GetPlayerState();
  CGraphics::SetCullMode(kCM_None);
  for (int i = 0; i < mModels.size(); ++i) {
    TToken< CModel > model(mModels[i]);
    const bool owned = playerState.HasPowerUp(static_cast< CPlayerState::EItemType >(i + CPlayerState::kIT_Truth));
    CColor color = skPostColor;
    if (owned) {
      if (ArtifactScanIds[i] != kInvalidAssetId) {
        const float scanTime = mgr.GetPlayerState()->GetScanTime(ArtifactScanIds[i]);
        const float interp = 2.f * (scanTime - 0.5f);
        if (interp < 0.5f) {
          color = CColor::Lerp(skPostColor, CColor::White(), 2.f * interp);
        } else {
          color = CColor::Lerp(CColor::White(), skPreColor, 2.f * (interp - 0.5f));
        }
      } else {
        color = skPreColor;
      }
    }
    if (inArtifactCategory && i == selectedArtifact) {
      const float interp = 0.5f * (1.f + CMath::SlowSineR(2.f * (M_PIF * CGraphics::GetSecondsMod900())));
      color = CColor::Lerp(CColor::White(), color, interp)
                  .WithAlphaModulatedBy(CMath::Clamp(0.f, 1.25f - interp, 1.f));
    }
    model->Draw(CModelFlags::Additive(0.f).DepthCompareUpdate(true, true));
    model->Draw(CModelFlags::AdditiveRGB(color.WithAlphaModulatedBy(effectiveAlpha)).DepthCompareUpdate(true, false));
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::DisableAllLights();
}

CAssetId CArtifactDoll::GetArtifactHeadScanFromItemType(CPlayerState::EItemType item) {
  if (item >= CPlayerState::kIT_Truth && item <= CPlayerState::kIT_Newborn) {
    int tmp = size_t(item) - CPlayerState::kIT_Truth;
    return ArtifactScanIds[tmp];
  }
  return kInvalidAssetId;
}

int CArtifactDoll::GetArtifactHeadScanIndex(CAssetId scanId) {
  for (size_t i = 0; i < ARRAY_SIZE(ArtifactScanIds); ++i) {
    if (ArtifactScanIds[i] == scanId) {
      return int(i);
    }
  }

  return -1;
}
