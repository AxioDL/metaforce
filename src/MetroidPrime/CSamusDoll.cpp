#include "MetroidPrime/CSamusDoll.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "rstl/math.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include <dolphin/gx/GXPixel.h>
#include <float.h>
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

static const int skSuitToCharacterType[8] = {0, 6, 2, 10, 16, 24, 20, 28};
static const int skSuitToCharacterType2[8][2] = {{14, 15}, {8, 9},   {4, 5},   {12, 13},
                                                 {18, 19}, {26, 27}, {22, 23}, {30, 31}};
static const int skSuitToBootsCharacterType[8] = {1, 7, 3, 11, 17, 25, 21, 29};
static const char* const skBeamModels[5] = {"CMDL_InvPowerBeam", "CMDL_InvIceBeam",
                                            "CMDL_InvWaveBeam", "CMDL_InvPlasmaBeam",
                                            "CMDL_InvPowerBeam"};
static const char* const skVisorModels[8] = {
    "CMDL_InvVisor",       "CMDL_InvGravityVisor", "CMDL_InvVisor",       "CMDL_InvPhazonVisor",
    "CMDL_InvFusionVisor", "CMDL_InvFusionVisor",  "CMDL_InvFusionVisor", "CMDL_InvFusionVisor"};
static const char* const skFinModels[8] = {
    "CMDL_InvPowerFins", "CMDL_InvPowerFins", "CMDL_InvPowerFins",   "CMDL_InvPowerFins",
    "CMDL_InvPowerFins", "CMDL_InvVariaFins", "CMDL_InvGravityFins", "CMDL_InvPhazonFins"};

static const char* const skGrappleModel = "CMDL_InvGrappleBeam";
static const char* const kGunLocator = "GUN_LCTR";
static const char* const kGrappleLocator = "GRAPPLE_LCTR";
static const char* const kVisorLocator = "VISOR_LCTR";
static const char* const skBallModel = "SamusBallANCS";

static const CVector3f skDefaultScale(1.f, 1.f, 1.f);
static const CVector3f skInitialOffset(0.f, 0.f, 0.8f);

CSamusDoll::CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
                       const CPlayerState::EPlayerSuit suit, const CPlayerState::EBeamId beam,
                       const bool hasSpiderBall, const bool hasGrappleBeam)
: mBallTransform(
      CTransform4f::Translate(0.f, 0.f, 0.625f * gpTweakPlayer->GetPlayerBallHalfExtent()))
, mAlphaIn(0.f)
, mSuit(suit)
, mBeam(beam)
, mCompletedMorphball(false)
, mSelectedMorphball(false)
, mTotalTransitionTime(1.f)
, mRemTransitionTime(0.f)
, mSuitPulseFactor(0.f)
, mBeamPulseFactor(0.f)
, mGrapplePulseFactor(0.f)
, mBootsPulseFactor(0.f)
, mVisorPulseFactor(0.f)
, mBallPulseFactor(0.f)
, mFixedRot(CQuaternion::ZRotation(CRelAngle::FromDegrees(180.f)))
, mFixedZoom(-3.6f)
, mInterpStartOffset(skInitialOffset)
, mUserInterpRot(mFixedRot)
, mUserInterpZoom(-3.6f)
, mOffset(skInitialOffset)
, mUserRot(mFixedRot)
, mUserZoom(-3.6f)
, mViewInterp(0.f)
, mSpiderBallGlass(gpSimplePool->GetObj(CMorphBall::skSpiderBallGlass[suit].mName))
, mBallMatIdx(hasSpiderBall ? CMorphBall::skSpiderBallCharacter[suit].mShader
                                : CMorphBall::skBallCharacter[suit].mShader)
, mGlassMatIdx(CMorphBall::skSpiderBallGlass[suit].mShader)
, mBallGlowColorIdx(hasSpiderBall ? CMorphBall::skSpiderBallGlowColorIdx[suit]
                                      : CMorphBall::skBallGlowColorIdx[suit])
, mItemScreenSamus(gpSimplePool->GetObj("ANCS_ItemScreenSamus"))
, mInvBeam(gpSimplePool->GetObj(skBeamModels[beam]))
, mInvVisor(gpSimplePool->GetObj(skVisorModels[suit]))
, mInvGrappleBeam(gpSimplePool->GetObj(skGrappleModel))
, mInvFins(gpSimplePool->GetObj(skFinModels[suit]))
, mBallInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, mBallInnerGlowGen(rs_new CElementGen(mBallInnerGlow))
, mBallTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, mBallTransitionFlashGen(nullptr)
, mLights(1, CLight::BuildDirectional(CVector3f::Forward(), CColor(0xffffffff)),
              rstl::rmemory_allocator())
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, mPhazonOffsetAngle(0.f)
, mHasSpiderBall(hasSpiderBall)
, mHasGrappleBeam(hasGrappleBeam)
, mPulseSuit(false)
, mPulseBeam(false)
, mPulseGrapple(false)
, mPulseBoots(false)
, mLoaded(false) {
  mBallInnerGlowGen->SetGlobalScale(0.625f * skDefaultScale);
  mSpiderBallGlass.Lock();
  mItemScreenSamus.Lock();
  mInvBeam.Lock();
  mInvVisor.Lock();
  mInvGrappleBeam.Lock();
  if (suit >= CPlayerState::kPS_FusionPower) {
    mInvFins.Lock();
  }
  mBallTransitionFlash.Lock();
  mDepTokens.reserve(suitDgrp.GetObjectTagVector().size() + ballDgrp.GetObjectTagVector().size());
  for (AUTO(it, suitDgrp.GetObjectTagVector().begin()); it != suitDgrp.GetObjectTagVector().end();
       ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    mDepTokens.push_back(token);
  }
  for (AUTO(it, ballDgrp.GetObjectTagVector().begin()); it != ballDgrp.GetObjectTagVector().end();
       ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    mDepTokens.push_back(token);
  }
}

bool CSamusDoll::CheckLoadComplete() {
  if (IsLoaded()) {
    return true;
  }
  for (AUTO(it, mDepTokens.begin()); it != mDepTokens.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  mSuitModel0 = BuildSuitModelData1(mSuit);
  for (int i = 0; i < 2; ++i) {
    const int character = skSuitToCharacterType2[mSuit][i];
    if (character != -1) {
      CAnimRes res(gpResourceFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->GetId(),
                   character, skDefaultScale, 2, true);
      CModelData model(res);
      mSuitModel1and2.push_back(model.AnimationData()->GetModelData());
    }
  }
  mSuitModelBoots = BuildSuitModelDataBoots(mSuit);
  const SMorphBallModelInfo* characters =
      mHasSpiderBall ? CMorphBall::skSpiderBallCharacter : CMorphBall::skBallCharacter;
  CAnimRes res(gpResourceFactory->GetResourceIdByName(characters[mSuit].mName)->GetId(), 0,
               skDefaultScale, 0, true);
  mBallModelData = CModelData(res);
  mBallMatIdx = characters[mSuit].mShader;
  mLoaded = true;
  return true;
}

bool CSamusDoll::IsLoaded() const {
  if (mLoaded) {
    return true;
  }
  if (!mItemScreenSamus.IsLoaded()) {
    return false;
  }
  if (!mInvBeam.GetObject()) {
    return false;
  }
  if (!mInvVisor.GetObject()) {
    return false;
  }
  if (!mInvGrappleBeam.GetObject()) {
    return false;
  }
  if (!mSpiderBallGlass.GetObject()) {
    return false;
  }
  if (mInvFins.IsLocked() && !mInvFins.GetObject()) {
    return false;
  }
  return mSuitModel0.valid();
}

CSamusDoll::~CSamusDoll() {
  SetOffsetSfxPlaying(false);
  SetZoomSfxPlaying(false);
  SetRotationSfxPlaying(false);
}

void CSamusDoll::SetupLights() {
  mLights[0] = CLight::BuildDirectional(mUserRot.BuildTransform().GetColumn(kDY),
                                            CColor(uchar(192), uchar(192), uchar(192), uchar(255)));
  mActorLights->BuildFakeLightList(mLights, CColor::Black());
}

CModelData CSamusDoll::BuildSuitModelData1(CPlayerState::EPlayerSuit suit) {
  CModelData model(CAnimRes(gpResourceFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->GetId(),
                            skSuitToCharacterType[suit], skDefaultScale, 2, true));
  CAnimPlaybackParms parms(2, -1, 1.f, true);
  model.AnimationData()->SetAnimation(parms, false);
  return model;
}

CModelData CSamusDoll::BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit) {
  CModelData model(CAnimRes(gpResourceFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->GetId(),
                            skSuitToBootsCharacterType[suit], skDefaultScale, 2, true));
  CAnimPlaybackParms parms(2, -1, 1.f, true);
  model.AnimationData()->SetAnimation(parms, false);
  return model;
}

void CSamusDoll::Touch() {
  if (!CheckLoadComplete()) {
    return;
  }
  mSuitModel0->AnimationData()->PreRender();
  mSuitModelBoots->AnimationData()->PreRender();
  mBallModelData->AnimationData()->PreRender();
  mSuitModel0->Touch(CModelData::kWM_Normal, 0);
  mSuitModelBoots->Touch(CModelData::kWM_Normal, 0);
  mBallModelData->Touch(CModelData::kWM_Normal, 0);
}

void CSamusDoll::Update(float dt, CRandom16& rand) {
  if (mInvBeam.TryCache()) {
    mInvBeam.GetObject()->Touch(0);
  }
  if (mInvVisor.TryCache()) {
    mInvVisor.GetObject()->Touch(0);
  }
  if (mInvGrappleBeam.TryCache()) {
    mInvGrappleBeam.GetObject()->Touch(0);
  }
  if (mSpiderBallGlass.TryCache()) {
    mSpiderBallGlass.GetObject()->Touch(0);
  }
  if (mInvFins.TryCache()) {
    mInvFins.GetObject()->Touch(0);
  }
  if (!CheckLoadComplete()) {
    return;
  }

  mAlphaIn = rstl::min_val(1.f, mAlphaIn + 2.f * dt);
  if (mRemTransitionTime > 0.f) {
    const float oldTime = mRemTransitionTime;
    mRemTransitionTime = rstl::max_val(0.f, mRemTransitionTime - dt);
    if (!mCompletedMorphball && mSelectedMorphball &&
        oldTime >= mTotalTransitionTime - 0.5f &&
        mRemTransitionTime < mTotalTransitionTime - 0.5f) {
      mBallTransitionFlashGen = rs_new CElementGen(mBallTransitionFlash);
      mBallTransitionFlashGen->SetGlobalScale(0.625f * skDefaultScale);
    }
    if (mRemTransitionTime == 0.f) {
      mCompletedMorphball = mSelectedMorphball;
      if (!mSelectedMorphball) {
        mSuitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
        mSuitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true),
                                                           false);
      }
    }
  }
  if (mPulseSuit) {
    mSuitPulseFactor = rstl::min_val(1.f, mSuitPulseFactor + 2.f * dt);
  } else {
    mSuitPulseFactor = rstl::max_val(0.f, mSuitPulseFactor - 2.f * dt);
  }
  if (mPulseBeam) {
    mBeamPulseFactor = rstl::min_val(1.f, mBeamPulseFactor + 2.f * dt);
  } else {
    mBeamPulseFactor = rstl::max_val(0.f, mBeamPulseFactor - 2.f * dt);
  }
  if (mPulseGrapple) {
    mGrapplePulseFactor = rstl::min_val(1.f, mGrapplePulseFactor + 2.f * dt);
  } else {
    mGrapplePulseFactor = rstl::max_val(0.f, mGrapplePulseFactor - 2.f * dt);
  }
  if (mPulseBoots) {
    mBootsPulseFactor = rstl::min_val(1.f, mBootsPulseFactor + 2.f * dt);
  } else {
    mBootsPulseFactor = rstl::max_val(0.f, mBootsPulseFactor - 2.f * dt);
  }
  if (mPulseVisor) {
    mVisorPulseFactor = rstl::min_val(1.f, mVisorPulseFactor + 2.f * dt);
  } else {
    mVisorPulseFactor = rstl::max_val(0.f, mVisorPulseFactor - 2.f * dt);
  }
  if (mCompletedMorphball) {
    mBallPulseFactor = rstl::min_val(1.f, mBallPulseFactor + 2.f * dt);
  } else {
    mBallPulseFactor = rstl::max_val(0.f, mBallPulseFactor - 2.f * dt);
  }

  if (mSuit == CPlayerState::kPS_Phazon) {
    if (!mPhazonIndirectTexture) {
      mPhazonIndirectTexture =
          TCachedToken< CTexture >(gpSimplePool->GetObj("PhazonIndirectTexture"));
      mPhazonIndirectTexture->Lock();
    }
  } else {
    mPhazonIndirectTexture = rstl::optional_object_null();
  }
  if (mPhazonIndirectTexture) {
    mPhazonIndirectTexture->TryCache();
    mPhazonOffsetAngle += 0.03f;
    mPhazonOffsetAngle = CMath::ClampRadians(mPhazonOffsetAngle);
    gpRender->AllocatePhazonSuitMaskTexture();
  }
  mSuitModel0->AdvanceAnimationIgnoreParticles(dt, rand, true);
  mSuitModelBoots->AdvanceAnimationIgnoreParticles(dt, rand, true);
  mBallModelData->AdvanceAnimationIgnoreParticles(dt, rand, true);
  SetupLights();
  mBallInnerGlowGen->SetGlobalTranslation(mBallTransform.GetTranslation());
  mBallInnerGlowGen->Update(dt);
  if (!mBallTransitionFlashGen.null()) {
    if (mBallTransitionFlashGen->IsSystemDeletable()) {
      mBallTransitionFlashGen = nullptr;
    }
    if (!mBallTransitionFlashGen.null()) {
      mBallTransitionFlashGen->SetGlobalTranslation(mBallTransform.GetTranslation());
      mBallTransitionFlashGen->Update(dt);
    }
  }
  if (mViewInterp != 0.f && mViewInterp != 1.f) {
    if (mViewInterp < 0.f) {
      mViewInterp = rstl::min_val(0.f, mViewInterp + 3.f * dt);
    } else {
      mViewInterp = rstl::min_val(1.f, mViewInterp + 3.f * dt);
    }
    const float interp = CMath::AbsF(mViewInterp);
    const float oneMinusInterp = 1.f - interp;
    mOffset = CVector3f::Lerp(skInitialOffset, mInterpStartOffset, interp);
    mUserRot =
        CQuaternion::Slerp(CQuaternion(CQuaternion::Dot(mUserInterpRot, mFixedRot) >= 0.f
                                           ? mFixedRot
                                           : mFixedRot.BuildEquivalent()),
                           mUserInterpRot, interp);
    if (mViewInterp <= 0.f) {
      mUserZoom = mUserInterpZoom * interp + mFixedZoom * oneMinusInterp;
    } else {
      mUserZoom = mUserInterpZoom * oneMinusInterp + mFixedZoom * interp;
    }
  }
}

void CSamusDoll::Draw(const CStateManager& mgr, float alpha) {
  if (!IsLoaded()) {
    return;
  }
  CModel* const beam = mInvBeam.GetObject();
  CModel* const visor = mInvVisor.GetObject();
  CModel* const grapple = mInvGrappleBeam.GetObject();
  CModel* const glass = mSpiderBallGlass.GetObject();
  CModel* const fins = mInvFins.GetObject();
  const float drawAlpha = alpha * mAlphaIn;
  float itemPulse =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900())), 1.f);
  itemPulse *= 1.f - CMath::AbsF(mViewInterp);
  gpRender->SetPerspective(55.f, static_cast< float >(CGraphics::GetViewportWidth()),
                           static_cast< float >(CGraphics::GetViewportHeight()), 0.2f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetViewPointMatrix(mUserRot.BuildTransform4f(mOffset) *
                                CTransform4f::Translate(0.f, mUserZoom, 0.f));
  const CTransform4f scale = CTransform4f::Scale(skDefaultScale);
  const CTransform4f gunXf =
      mSuitModel0->GetScaledLocatorTransform(rstl::string_l(kGunLocator)) * scale;
  const CTransform4f visorXf =
      mSuitModel0->GetScaledLocatorTransform(rstl::string_l(kVisorLocator)) * scale;
  const CTransform4f grappleXf =
      mSuitModel0->GetScaledLocatorTransform(rstl::string_l(kGrappleLocator)) * scale;
  const bool completedMorphball = mCompletedMorphball;
  const float suitPulse = itemPulse * mSuitPulseFactor;
  if (!completedMorphball || !mSelectedMorphball) {
    const float bootsPulse = rstl::max_val(suitPulse, itemPulse * mBootsPulseFactor);
    const bool phazonSuit = mSuit == CPlayerState::kPS_Phazon;
    if (phazonSuit) {
      GXSetDstAlpha(GX_TRUE, 255);
    }
    for (int i = 0; i <= mSuitModel1and2.size(); ++i) {
      TLockedToken< CSkinnedModel > backupModel = mSuitModel0->AnimationData()->GetModelData();
      if (i < mSuitModel1and2.size()) {
        mSuitModel0->AnimationData()->SubstituteModelData(mSuitModel1and2[i]);
      }
      mSuitModel0->MultiLightingDraw(CModelData::kWM_Normal, CTransform4f::Identity(),
                                        mActorLights.get(),
                                        CColor::White().WithAlphaOf(drawAlpha),
                                        CColor::White().WithAlphaOf(drawAlpha * suitPulse));
      mSuitModel0->AnimationData()->SubstituteModelData(backupModel);
    }
    mSuitModelBoots->MultiLightingDraw(CModelData::kWM_Normal, CTransform4f::Identity(),
                                           mActorLights.get(),
                                           CColor::White().WithAlphaOf(drawAlpha),
                                           CColor::White().WithAlphaOf(drawAlpha * bootsPulse));
    CGraphics::LoadLight(kLight0, mLights[0]);
    CGraphics::EnableLight(kLight0);
    CGraphics::SetAmbientColor(CColor(0x00000000));
    CGraphics::SetModelMatrix(gunXf);
    beam->Draw(CModelFlags::AlphaBlended(drawAlpha));
    beam->Draw(CModelFlags::Additive(drawAlpha * itemPulse * mBeamPulseFactor)
                   .DepthCompareUpdate(true, false));

    CGraphics::SetModelMatrix(visorXf);
    const float visorT =
        CMath::ModF(CGraphics::GetSecondsMod900(), 1.f) * (1.f - CMath::AbsF(mViewInterp));
    const float alphaBlend = visorT < 0.25f  ? 1.f - 2.f * visorT
                             : visorT < 0.5f ? 2.f * (visorT - 0.25f) + 0.5f
                                             : 1.f;
    const float addBlend = visorT > 0.75f  ? 1.f - 4.f * (visorT - 0.75f)
                           : visorT > 0.5f ? 4.f * (visorT - 0.5f)
                                           : 0.f;
    const CColor visorColor =
        CColor::Lerp(CColor::White().WithAlphaOf(drawAlpha),
                     CColor(alphaBlend, alphaBlend, alphaBlend, drawAlpha), mVisorPulseFactor);
    visor->Draw(CModelFlags::AlphaBlended(visorColor));
    visor->Draw(CModelFlags::Additive(drawAlpha * addBlend * mVisorPulseFactor)
                    .DepthCompareUpdate(true, false));
    if (mHasGrappleBeam) {
      CGraphics::SetModelMatrix(grappleXf);
      grapple->Draw(CModelFlags::AlphaBlended(drawAlpha));
      grapple->Draw(CModelFlags::Additive(drawAlpha * itemPulse * mGrapplePulseFactor)
                        .DepthCompareUpdate(true, false));
    } else if (mSuit >= CPlayerState::kPS_FusionPower) {
      CGraphics::SetModelMatrix(grappleXf);
      fins->Draw(CModelFlags::AlphaBlended(drawAlpha));
      fins->Draw(CModelFlags::Additive(drawAlpha * suitPulse).DepthCompareUpdate(true, false));
    }
    if (mRemTransitionTime > 0.f) {
      const float ballT = 1.f - mRemTransitionTime / mTotalTransitionTime;
      float ballAlpha = 0.f;
      if (mSelectedMorphball) {
        ballAlpha = 1.f - rstl::min_val(1.f, mRemTransitionTime / 0.25f);
      } else if (mCompletedMorphball) {
        ballAlpha =
            rstl::max_val(0.f, (mRemTransitionTime - (mTotalTransitionTime - 0.25f)) / 0.25f);
      }
      if (ballAlpha > 0.f) {
        CModelFlags flags[2] = {
            CModelFlags::AlphaBlended(0.f)
                .DepthCompareUpdate(true, true)
                .UseShaderSet(mBallMatIdx),
            CModelFlags::AlphaBlended(ballAlpha * drawAlpha).UseShaderSet(mBallMatIdx)};
        mBallModelData->MultipassDraw(CModelData::kWM_Normal, mBallTransform,
                                          mActorLights.get(), flags, 2);
        mBallModelData->Render(
            mgr, mBallTransform, nullptr,
            CModelFlags::Additive(mBallPulseFactor * (itemPulse * (ballAlpha * drawAlpha)))
                .UseShaderSet(mBallMatIdx));
      }
      if (mSelectedMorphball && ballT > 0.5f) {
        const float ballEndT = (ballT - 0.5f) / 0.5f;
        const float oneMinusBallEndT = 1.f - ballEndT;
        const float spinScale = 0.75f * oneMinusBallEndT + 1.f;
        float spinAlpha;
        if (ballEndT < 0.1f) {
          spinAlpha = 0.f;
        } else if (ballEndT < 0.2f) {
          spinAlpha = (ballEndT - 0.1f) / 0.1f;
        } else if (ballEndT < 0.9f) {
          spinAlpha = 1.f;
        } else {
          spinAlpha = 1.f - (ballT - 0.9f) / (1.f - 0.9f);
        }
        spinAlpha *= 0.5f;
        const CRelAngle spinAngle = CRelAngle::FromDegrees(360.f * oneMinusBallEndT);
        if (spinAlpha > 0.f) {
          mBallModelData->Render(mgr,
                                     mBallTransform * CTransform4f::RotateZ(spinAngle) *
                                         CTransform4f::Scale(spinScale, spinScale, spinScale),
                                     mActorLights.get(),
                                     CModelFlags::Additive(spinAlpha * drawAlpha)
                                         .DepthCompareUpdate(true, false)
                                         .UseShaderSet(mBallMatIdx));
        }
      }
      if (mHasSpiderBall && ballAlpha > 0.f) {
        CGraphics::SetModelMatrix(mBallTransform);
        glass->Draw(CModelFlags::AlphaBlended(0.f)
                        .DepthCompareUpdate(true, true)
                        .UseShaderSet(mGlassMatIdx));
        glass->Draw(
            CModelFlags::AlphaBlended(ballAlpha * drawAlpha).UseShaderSet(mGlassMatIdx));
        glass->Draw(
            CModelFlags::Additive(mBallPulseFactor * (itemPulse * (ballAlpha * drawAlpha)))
                .UseShaderSet(mGlassMatIdx));
      }
    }
    if (phazonSuit && drawAlpha > 0.1f) {
      CCubeRenderer* const renderer = gpRender;
      const float radius = CMath::Clamp(
          0.2f, (10.f - (mUserZoom >= 0.f ? mUserZoom : -mUserZoom)) / 20.f, 1.f);
      renderer->DrawPhazonSuitIndirectEffect(
          CColor(0.1f, 0.1f, 0.1f, drawAlpha), mPhazonIndirectTexture,
          radius, 0.1f, CMath::FastSinR(mPhazonOffsetAngle),
          CMath::FastSinR(mPhazonOffsetAngle),
          gpTweakGuiColors->GetPauseBlurFilterColor().WithAlphaOf(drawAlpha));
    }
  } else {
    CModelFlags flags[2] = {
        CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, true).UseShaderSet(mBallMatIdx),
        CModelFlags::AlphaBlended(drawAlpha).UseShaderSet(mBallMatIdx)};
    mBallModelData->MultipassDraw(CModelData::kWM_Normal, mBallTransform,
                                      mActorLights.get(), flags, 2);
    mBallModelData->Render(mgr, mBallTransform, nullptr,
                               CModelFlags::Additive(mBallPulseFactor * (drawAlpha * itemPulse))
                                   .UseShaderSet(mBallMatIdx));
    const CMorphBall::SColorRgb& glow = CMorphBall::skBallInnerGlowColors[mBallGlowColorIdx];
    mBallInnerGlowGen->SetModulationColor(
        CColor(glow.mR, glow.mG, glow.mB).WithAlphaOf(drawAlpha));
    if (drawAlpha > 0.f) {
      if (mBallInnerGlowGen->GetNumActiveChildParticles() > 0) {
        CParticleGen* child = mBallInnerGlowGen->GetActiveChildParticle(0);
        const CMorphBall::SColorRgb& hull = CMorphBall::skBallHullGlowColors[mBallGlowColorIdx];
        child->SetModulationColor(CColor(hull.mR, hull.mG, hull.mB).WithAlphaOf(drawAlpha));
        if (mBallInnerGlowGen->GetNumActiveChildParticles() > 1) {
          CParticleGen* child = mBallInnerGlowGen->GetActiveChildParticle(1);
          const CMorphBall::SColorRgb& aux =
              CMorphBall::skBallBoostedHullGlowColors[mBallGlowColorIdx];
          child->SetModulationColor(CColor(aux.mR, aux.mG, aux.mB).WithAlphaOf(drawAlpha));
        }
      }
      mBallInnerGlowGen->Render();
    }
    if (mHasSpiderBall) {
      CGraphics::SetModelMatrix(mBallTransform);
      glass->Draw(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, true));
      glass->Draw(CModelFlags::AlphaBlended(drawAlpha).UseShaderSet(mGlassMatIdx));
      glass->Draw(CModelFlags::Additive(mBallPulseFactor * (drawAlpha * itemPulse))
                      .UseShaderSet(mGlassMatIdx));
    }
  }
  if (!mBallTransitionFlashGen.null()) {
    const CMorphBall::SColorRgb& color = CMorphBall::skBallHullGlowColors[mBallGlowColorIdx];
    mBallTransitionFlashGen->SetModulationColor(CColor(color.mR, color.mG, color.mB));
    mBallTransitionFlashGen->Render();
  }
  CGraphics::DisableAllLights();
}

void CSamusDoll::CheckTransition(bool morphball) {
  if (mRemTransitionTime > 0.f || morphball == mSelectedMorphball) {
    return;
  }
  mSelectedMorphball = morphball;
  SetTransitionAnimation();
}

void CSamusDoll::SetTransitionAnimation() {
  if (!mCompletedMorphball) {
    mSuitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
    mSuitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
    mRemTransitionTime = mTotalTransitionTime =
        mSuitModel0->AnimationData()->GetAnimationDuration(0);
  } else if (!mSelectedMorphball) {
    mSuitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(1, -1, 1.f, true), false);
    mSuitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(1, -1, 1.f, true), false);
    mRemTransitionTime = mTotalTransitionTime =
        mSuitModel0->AnimationData()->GetAnimationDuration(1);
  }
}

void CSamusDoll::SetPulseBeam(bool pulse) { mPulseBeam = pulse; }
void CSamusDoll::SetPulseGrapple(bool pulse) { mPulseGrapple = pulse; }
void CSamusDoll::SetPulseBoots(bool pulse) { mPulseBoots = pulse; }
void CSamusDoll::SetPulseVisor(bool pulse) { mPulseVisor = pulse; }
void CSamusDoll::SetPulseSuit(bool pulse) { mPulseSuit = pulse; }

void CSamusDoll::SetRotation(float xDelta, float zDelta, float dt) {
  if (IsViewInterpolating()) {
    return;
  }
  SetRotationSfxPlaying(xDelta != 0.f || zDelta != 0.f);
  CEulerAngles angles = CEulerAngles::FromQuaternion(mUserRot);
  angles.SetX(CMath::ClampRadians(angles.GetX()));
  angles.SetZ(CMath::ClampRadians(angles.GetZ()));
  const float& oldX = angles.GetX();
  angles.SetX(oldX + xDelta);
  angles.SetX(CMath::ClampRadians(angles.GetX()));
  const float& oldZ = angles.GetZ();
  angles.SetZ(oldZ + zDelta);
  angles.SetZ(CMath::ClampRadians(angles.GetZ()));
  float minAngle = -0.99f * M_PIF / 2.f;
  float maxAngle = 0.99f * M_PIF / 2.f;
  float newAngleX = angles.GetX();
  if (newAngleX > M_PIF) {
    newAngleX -= M_2PIF;
  }
  const float clampedX = CMath::ClampRadians(CMath::Clamp(minAngle, newAngleX, maxAngle));
  mUserRot =
      CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(clampedX),
                               CRelAngle::FromRadians(angles.GetZ()));
}

void CSamusDoll::SetOffset(const CVector3f& offset, float dt) {
  if (IsViewInterpolating()) {
    return;
  }
  CVector3f oldOffset = mOffset;
  const CVector3f move(offset.GetX(), 0.f, offset.GetZ());
  const CMatrix3f rotation = mUserRot.BuildTransform();
  const CVector3f rotatedMove = rotation * move;
  mOffset += rotatedMove;
  if ((oldOffset - mOffset).Magnitude() > dt) {
    SetOffsetSfxPlaying(true);
  } else {
    SetOffsetSfxPlaying(false);
  }
  const float oldZoom = mUserZoom;
  static const float skMinZoom = -4.f;
  static const float skMaxZoom = -2.2f;
  mUserZoom = CMath::Clamp(skMinZoom, mUserZoom + offset.GetY(), skMaxZoom);
  bool zoomSfx = false;
  if (CMath::AbsF(mUserZoom - oldZoom) > dt) {
    zoomSfx = true;
  }
  const float zoomDelta = offset.GetY() - (mUserZoom - oldZoom);
  const CVector3f newOffset = mOffset + rotation.GetColumn(kDY) * zoomDelta;
  CVector3f delta = newOffset - mOffset;
  const CVector3f beforeZoomOffset = mOffset;
  if (!(delta == CVector3f::Zero())) {
    float time;
    CVector3f point = CVector3f::Zero();
    if (CollisionUtil::RaySphereIntersection(CSphere(skInitialOffset, 1.f), mOffset,
                                             delta.AsNormalized(), 0.f, time, point)) {
      const float pointDistance = (point - mOffset).Magnitude();
      if (pointDistance < CMath::AbsF(zoomDelta)) {
        mOffset = point;
      } else {
        mOffset = newOffset;
      }
    } else {
      mOffset = newOffset;
    }
  }
  if ((beforeZoomOffset - mOffset).Magnitude() > dt) {
    zoomSfx = true;
  }
  SetZoomSfxPlaying(zoomSfx);
  const CVector3f offsetDelta = mOffset - skInitialOffset;
  if (offsetDelta.Magnitude() > 1.f) {
    mOffset = skInitialOffset + 1.f * offsetDelta.AsNormalized();
  }
}

void CSamusDoll::BeginViewInterpolate(bool zoomIn) {
  if (mViewInterp == 0.f) {
    CSfxManager::SfxStart(0x5a0);
  } else if (mViewInterp == 1.f) {
    SetOffsetSfxPlaying(false);
    SetZoomSfxPlaying(false);
    SetRotationSfxPlaying(false);
    CSfxManager::SfxStart(0x5a1);
  } else {
    return;
  }
  mViewInterp = zoomIn ? FLT_EPSILON : -1.f + FLT_EPSILON;
  mInterpStartOffset = mOffset;
  mUserInterpRot = mUserRot;
  mUserInterpZoom = mUserZoom;
  mFixedZoom = zoomIn ? -2.2f : -3.6f;
}

void CSamusDoll::SetOffsetSfxPlaying(bool playing) {
  if (playing) {
    if (!mOffsetSfx) {
      mOffsetSfx =
          CSfxManager::SfxStart(0x57e, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(mOffsetSfx);
    mOffsetSfx.Clear();
  }
}

void CSamusDoll::SetZoomSfxPlaying(bool playing) {
  if (playing) {
    if (!mZoomSfx) {
      mZoomSfx = CSfxManager::SfxStart(0x560, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(mZoomSfx);
    mZoomSfx.Clear();
  }
}

void CSamusDoll::SetRotationSfxPlaying(bool playing) {
  if (playing) {
    if (!mRotateSfx) {
      mRotateSfx =
          CSfxManager::SfxStart(0x55f, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(mRotateSfx);
    mRotateSfx.Clear();
  }
}
