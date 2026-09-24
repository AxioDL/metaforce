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
: x10_ballTransform(
      CTransform4f::Translate(0.f, 0.f, 0.625f * gpTweakPlayer->GetPlayerBallHalfExtent()))
, x40_alphaIn(0.f)
, x44_suit(suit)
, x48_beam(beam)
, x4c_completedMorphball(false)
, x4d_selectedMorphball(false)
, x50_totalTransitionTime(1.f)
, x54_remTransitionTime(0.f)
, x58_suitPulseFactor(0.f)
, x5c_beamPulseFactor(0.f)
, x60_grapplePulseFactor(0.f)
, x64_bootsPulseFactor(0.f)
, x68_visorPulseFactor(0.f)
, x6c_ballPulseFactor(0.f)
, x70_fixedRot(CQuaternion::ZRotation(CRelAngle::FromDegrees(180.f)))
, x80_fixedZoom(-3.6f)
, x84_interpStartOffset(skInitialOffset)
, x90_userInterpRot(x70_fixedRot)
, xa0_userInterpZoom(-3.6f)
, xa4_offset(skInitialOffset)
, xb0_userRot(x70_fixedRot)
, xc0_userZoom(-3.6f)
, xc4_viewInterp(0.f)
, x1d4_spiderBallGlass(gpSimplePool->GetObj(CMorphBall::skSpiderBallGlass[suit].x0_name))
, x1e0_ballMatIdx(hasSpiderBall ? CMorphBall::skSpiderBallCharacter[suit].x4_shader
                                : CMorphBall::skBallCharacter[suit].x4_shader)
, x1e4_glassMatIdx(CMorphBall::skSpiderBallGlass[suit].x4_shader)
, x1e8_ballGlowColorIdx(hasSpiderBall ? CMorphBall::skSpiderBallGlowColorIdx[suit]
                                      : CMorphBall::skBallGlowColorIdx[suit])
, x1ec_itemScreenSamus(gpSimplePool->GetObj("ANCS_ItemScreenSamus"))
, x1f4_invBeam(gpSimplePool->GetObj(skBeamModels[beam]))
, x200_invVisor(gpSimplePool->GetObj(skVisorModels[suit]))
, x20c_invGrappleBeam(gpSimplePool->GetObj(skGrappleModel))
, x218_invFins(gpSimplePool->GetObj(skFinModels[suit]))
, x224_ballInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, x22c_ballInnerGlowGen(rs_new CElementGen(x224_ballInnerGlow))
, x230_ballTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, x238_ballTransitionFlashGen(nullptr)
, x23c_lights(1, CLight::BuildDirectional(CVector3f::Forward(), CColor(0xffffffff)),
              rstl::rmemory_allocator())
, x24c_actorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, x260_phazonOffsetAngle(0.f)
, x270_24_hasSpiderBall(hasSpiderBall)
, x270_25_hasGrappleBeam(hasGrappleBeam)
, x270_26_pulseSuit(false)
, x270_27_pulseBeam(false)
, x270_28_pulseGrapple(false)
, x270_29_pulseBoots(false)
, x270_31_loaded(false) {
  x22c_ballInnerGlowGen->SetGlobalScale(0.625f * skDefaultScale);
  x1d4_spiderBallGlass.Lock();
  x1ec_itemScreenSamus.Lock();
  x1f4_invBeam.Lock();
  x200_invVisor.Lock();
  x20c_invGrappleBeam.Lock();
  if (suit >= CPlayerState::kPS_FusionPower) {
    x218_invFins.Lock();
  }
  x230_ballTransitionFlash.Lock();
  x0_depTokens.reserve(suitDgrp.GetObjectTagVector().size() + ballDgrp.GetObjectTagVector().size());
  for (AUTO(it, suitDgrp.GetObjectTagVector().begin()); it != suitDgrp.GetObjectTagVector().end();
       ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    x0_depTokens.push_back(token);
  }
  for (AUTO(it, ballDgrp.GetObjectTagVector().begin()); it != ballDgrp.GetObjectTagVector().end();
       ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    x0_depTokens.push_back(token);
  }
}

bool CSamusDoll::CheckLoadComplete() {
  if (IsLoaded()) {
    return true;
  }
  for (AUTO(it, x0_depTokens.begin()); it != x0_depTokens.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  xc8_suitModel0 = BuildSuitModelData1(x44_suit);
  for (int i = 0; i < 2; ++i) {
    const int character = skSuitToCharacterType2[x44_suit][i];
    if (character != -1) {
      CAnimRes res(gpResourceFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->GetId(),
                   character, skDefaultScale, 2, true);
      CModelData model(res);
      x118_suitModel1and2.push_back(model.AnimationData()->GetModelData());
    }
  }
  x134_suitModelBoots = BuildSuitModelDataBoots(x44_suit);
  const SMorphBallModelInfo* characters =
      x270_24_hasSpiderBall ? CMorphBall::skSpiderBallCharacter : CMorphBall::skBallCharacter;
  CAnimRes res(gpResourceFactory->GetResourceIdByName(characters[x44_suit].x0_name)->GetId(), 0,
               skDefaultScale, 0, true);
  x184_ballModelData = CModelData(res);
  x1e0_ballMatIdx = characters[x44_suit].x4_shader;
  x270_31_loaded = true;
  return true;
}

bool CSamusDoll::IsLoaded() const {
  if (x270_31_loaded) {
    return true;
  }
  if (!x1ec_itemScreenSamus.IsLoaded()) {
    return false;
  }
  if (!x1f4_invBeam.GetObject()) {
    return false;
  }
  if (!x200_invVisor.GetObject()) {
    return false;
  }
  if (!x20c_invGrappleBeam.GetObject()) {
    return false;
  }
  if (!x1d4_spiderBallGlass.GetObject()) {
    return false;
  }
  if (x218_invFins.IsLocked() && !x218_invFins.GetObject()) {
    return false;
  }
  return xc8_suitModel0.valid();
}

CSamusDoll::~CSamusDoll() {
  SetOffsetSfxPlaying(false);
  SetZoomSfxPlaying(false);
  SetRotationSfxPlaying(false);
}

void CSamusDoll::SetupLights() {
  x23c_lights[0] = CLight::BuildDirectional(xb0_userRot.BuildTransform().GetColumn(kDY),
                                            CColor(uchar(192), uchar(192), uchar(192), uchar(255)));
  x24c_actorLights->BuildFakeLightList(x23c_lights, CColor::Black());
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
  xc8_suitModel0->AnimationData()->PreRender();
  x134_suitModelBoots->AnimationData()->PreRender();
  x184_ballModelData->AnimationData()->PreRender();
  xc8_suitModel0->Touch(CModelData::kWM_Normal, 0);
  x134_suitModelBoots->Touch(CModelData::kWM_Normal, 0);
  x184_ballModelData->Touch(CModelData::kWM_Normal, 0);
}

void CSamusDoll::Update(float dt, CRandom16& rand) {
  if (x1f4_invBeam.TryCache()) {
    x1f4_invBeam.GetObject()->Touch(0);
  }
  if (x200_invVisor.TryCache()) {
    x200_invVisor.GetObject()->Touch(0);
  }
  if (x20c_invGrappleBeam.TryCache()) {
    x20c_invGrappleBeam.GetObject()->Touch(0);
  }
  if (x1d4_spiderBallGlass.TryCache()) {
    x1d4_spiderBallGlass.GetObject()->Touch(0);
  }
  if (x218_invFins.TryCache()) {
    x218_invFins.GetObject()->Touch(0);
  }
  if (!CheckLoadComplete()) {
    return;
  }

  x40_alphaIn = rstl::min_val(1.f, x40_alphaIn + 2.f * dt);
  if (x54_remTransitionTime > 0.f) {
    const float oldTime = x54_remTransitionTime;
    x54_remTransitionTime = rstl::max_val(0.f, x54_remTransitionTime - dt);
    if (!x4c_completedMorphball && x4d_selectedMorphball &&
        oldTime >= x50_totalTransitionTime - 0.5f &&
        x54_remTransitionTime < x50_totalTransitionTime - 0.5f) {
      x238_ballTransitionFlashGen = rs_new CElementGen(x230_ballTransitionFlash);
      x238_ballTransitionFlashGen->SetGlobalScale(0.625f * skDefaultScale);
    }
    if (x54_remTransitionTime == 0.f) {
      x4c_completedMorphball = x4d_selectedMorphball;
      if (!x4d_selectedMorphball) {
        xc8_suitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
        x134_suitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true),
                                                           false);
      }
    }
  }
  if (x270_26_pulseSuit) {
    x58_suitPulseFactor = rstl::min_val(1.f, x58_suitPulseFactor + 2.f * dt);
  } else {
    x58_suitPulseFactor = rstl::max_val(0.f, x58_suitPulseFactor - 2.f * dt);
  }
  if (x270_27_pulseBeam) {
    x5c_beamPulseFactor = rstl::min_val(1.f, x5c_beamPulseFactor + 2.f * dt);
  } else {
    x5c_beamPulseFactor = rstl::max_val(0.f, x5c_beamPulseFactor - 2.f * dt);
  }
  if (x270_28_pulseGrapple) {
    x60_grapplePulseFactor = rstl::min_val(1.f, x60_grapplePulseFactor + 2.f * dt);
  } else {
    x60_grapplePulseFactor = rstl::max_val(0.f, x60_grapplePulseFactor - 2.f * dt);
  }
  if (x270_29_pulseBoots) {
    x64_bootsPulseFactor = rstl::min_val(1.f, x64_bootsPulseFactor + 2.f * dt);
  } else {
    x64_bootsPulseFactor = rstl::max_val(0.f, x64_bootsPulseFactor - 2.f * dt);
  }
  if (x270_30_pulseVisor) {
    x68_visorPulseFactor = rstl::min_val(1.f, x68_visorPulseFactor + 2.f * dt);
  } else {
    x68_visorPulseFactor = rstl::max_val(0.f, x68_visorPulseFactor - 2.f * dt);
  }
  if (x4c_completedMorphball) {
    x6c_ballPulseFactor = rstl::min_val(1.f, x6c_ballPulseFactor + 2.f * dt);
  } else {
    x6c_ballPulseFactor = rstl::max_val(0.f, x6c_ballPulseFactor - 2.f * dt);
  }

  if (x44_suit == CPlayerState::kPS_Phazon) {
    if (!x250_phazonIndirectTexture) {
      x250_phazonIndirectTexture =
          TCachedToken< CTexture >(gpSimplePool->GetObj("PhazonIndirectTexture"));
      x250_phazonIndirectTexture->Lock();
    }
  } else {
    x250_phazonIndirectTexture = rstl::optional_object_null();
  }
  if (x250_phazonIndirectTexture) {
    x250_phazonIndirectTexture->TryCache();
    x260_phazonOffsetAngle += 0.03f;
    x260_phazonOffsetAngle = CMath::ClampRadians(x260_phazonOffsetAngle);
    gpRender->AllocatePhazonSuitMaskTexture();
  }
  xc8_suitModel0->AdvanceAnimationIgnoreParticles(dt, rand, true);
  x134_suitModelBoots->AdvanceAnimationIgnoreParticles(dt, rand, true);
  x184_ballModelData->AdvanceAnimationIgnoreParticles(dt, rand, true);
  SetupLights();
  x22c_ballInnerGlowGen->SetGlobalTranslation(x10_ballTransform.GetTranslation());
  x22c_ballInnerGlowGen->Update(dt);
  if (!x238_ballTransitionFlashGen.null()) {
    if (x238_ballTransitionFlashGen->IsSystemDeletable()) {
      x238_ballTransitionFlashGen = nullptr;
    }
    if (!x238_ballTransitionFlashGen.null()) {
      x238_ballTransitionFlashGen->SetGlobalTranslation(x10_ballTransform.GetTranslation());
      x238_ballTransitionFlashGen->Update(dt);
    }
  }
  if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f) {
    if (xc4_viewInterp < 0.f) {
      xc4_viewInterp = rstl::min_val(0.f, xc4_viewInterp + 3.f * dt);
    } else {
      xc4_viewInterp = rstl::min_val(1.f, xc4_viewInterp + 3.f * dt);
    }
    const float interp = CMath::AbsF(xc4_viewInterp);
    const float oneMinusInterp = 1.f - interp;
    xa4_offset = CVector3f::Lerp(skInitialOffset, x84_interpStartOffset, interp);
    xb0_userRot =
        CQuaternion::Slerp(CQuaternion(CQuaternion::Dot(x90_userInterpRot, x70_fixedRot) >= 0.f
                                           ? x70_fixedRot
                                           : x70_fixedRot.BuildEquivalent()),
                           x90_userInterpRot, interp);
    if (xc4_viewInterp <= 0.f) {
      xc0_userZoom = xa0_userInterpZoom * interp + x80_fixedZoom * oneMinusInterp;
    } else {
      xc0_userZoom = xa0_userInterpZoom * oneMinusInterp + x80_fixedZoom * interp;
    }
  }
}

void CSamusDoll::Draw(const CStateManager& mgr, float alpha) {
  if (!IsLoaded()) {
    return;
  }
  CModel* const beam = x1f4_invBeam.GetObject();
  CModel* const visor = x200_invVisor.GetObject();
  CModel* const grapple = x20c_invGrappleBeam.GetObject();
  CModel* const glass = x1d4_spiderBallGlass.GetObject();
  CModel* const fins = x218_invFins.GetObject();
  const float drawAlpha = alpha * x40_alphaIn;
  float itemPulse =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900())), 1.f);
  itemPulse *= 1.f - CMath::AbsF(xc4_viewInterp);
  gpRender->SetPerspective(55.f, static_cast< float >(CGraphics::GetViewportWidth()),
                           static_cast< float >(CGraphics::GetViewportHeight()), 0.2f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetViewPointMatrix(xb0_userRot.BuildTransform4f(xa4_offset) *
                                CTransform4f::Translate(0.f, xc0_userZoom, 0.f));
  const CTransform4f scale = CTransform4f::Scale(skDefaultScale);
  const CTransform4f gunXf =
      xc8_suitModel0->GetScaledLocatorTransform(rstl::string_l(kGunLocator)) * scale;
  const CTransform4f visorXf =
      xc8_suitModel0->GetScaledLocatorTransform(rstl::string_l(kVisorLocator)) * scale;
  const CTransform4f grappleXf =
      xc8_suitModel0->GetScaledLocatorTransform(rstl::string_l(kGrappleLocator)) * scale;
  const bool completedMorphball = x4c_completedMorphball;
  const float suitPulse = itemPulse * x58_suitPulseFactor;
  if (!completedMorphball || !x4d_selectedMorphball) {
    const float bootsPulse = rstl::max_val(suitPulse, itemPulse * x64_bootsPulseFactor);
    const bool phazonSuit = x44_suit == CPlayerState::kPS_Phazon;
    if (phazonSuit) {
      GXSetDstAlpha(GX_TRUE, 255);
    }
    for (int i = 0; i <= x118_suitModel1and2.size(); ++i) {
      TLockedToken< CSkinnedModel > backupModel = xc8_suitModel0->AnimationData()->GetModelData();
      if (i < x118_suitModel1and2.size()) {
        xc8_suitModel0->AnimationData()->SubstituteModelData(x118_suitModel1and2[i]);
      }
      xc8_suitModel0->MultiLightingDraw(CModelData::kWM_Normal, CTransform4f::Identity(),
                                        x24c_actorLights.get(),
                                        CColor::White().WithAlphaOf(drawAlpha),
                                        CColor::White().WithAlphaOf(drawAlpha * suitPulse));
      xc8_suitModel0->AnimationData()->SubstituteModelData(backupModel);
    }
    x134_suitModelBoots->MultiLightingDraw(CModelData::kWM_Normal, CTransform4f::Identity(),
                                           x24c_actorLights.get(),
                                           CColor::White().WithAlphaOf(drawAlpha),
                                           CColor::White().WithAlphaOf(drawAlpha * bootsPulse));
    CGraphics::LoadLight(kLight0, x23c_lights[0]);
    CGraphics::EnableLight(kLight0);
    CGraphics::SetAmbientColor(CColor(0x00000000));
    CGraphics::SetModelMatrix(gunXf);
    beam->Draw(CModelFlags::AlphaBlended(drawAlpha));
    beam->Draw(CModelFlags::Additive(drawAlpha * itemPulse * x5c_beamPulseFactor)
                   .DepthCompareUpdate(true, false));

    CGraphics::SetModelMatrix(visorXf);
    const float visorT =
        CMath::ModF(CGraphics::GetSecondsMod900(), 1.f) * (1.f - CMath::AbsF(xc4_viewInterp));
    const float alphaBlend = visorT < 0.25f  ? 1.f - 2.f * visorT
                             : visorT < 0.5f ? 2.f * (visorT - 0.25f) + 0.5f
                                             : 1.f;
    const float addBlend = visorT > 0.75f  ? 1.f - 4.f * (visorT - 0.75f)
                           : visorT > 0.5f ? 4.f * (visorT - 0.5f)
                                           : 0.f;
    const CColor visorColor =
        CColor::Lerp(CColor::White().WithAlphaOf(drawAlpha),
                     CColor(alphaBlend, alphaBlend, alphaBlend, drawAlpha), x68_visorPulseFactor);
    visor->Draw(CModelFlags::AlphaBlended(visorColor));
    visor->Draw(CModelFlags::Additive(drawAlpha * addBlend * x68_visorPulseFactor)
                    .DepthCompareUpdate(true, false));
    if (x270_25_hasGrappleBeam) {
      CGraphics::SetModelMatrix(grappleXf);
      grapple->Draw(CModelFlags::AlphaBlended(drawAlpha));
      grapple->Draw(CModelFlags::Additive(drawAlpha * itemPulse * x60_grapplePulseFactor)
                        .DepthCompareUpdate(true, false));
    } else if (x44_suit >= CPlayerState::kPS_FusionPower) {
      CGraphics::SetModelMatrix(grappleXf);
      fins->Draw(CModelFlags::AlphaBlended(drawAlpha));
      fins->Draw(CModelFlags::Additive(drawAlpha * suitPulse).DepthCompareUpdate(true, false));
    }
    if (x54_remTransitionTime > 0.f) {
      const float ballT = 1.f - x54_remTransitionTime / x50_totalTransitionTime;
      float ballAlpha = 0.f;
      if (x4d_selectedMorphball) {
        ballAlpha = 1.f - rstl::min_val(1.f, x54_remTransitionTime / 0.25f);
      } else if (x4c_completedMorphball) {
        ballAlpha =
            rstl::max_val(0.f, (x54_remTransitionTime - (x50_totalTransitionTime - 0.25f)) / 0.25f);
      }
      if (ballAlpha > 0.f) {
        CModelFlags flags[2] = {
            CModelFlags::AlphaBlended(0.f)
                .DepthCompareUpdate(true, true)
                .UseShaderSet(x1e0_ballMatIdx),
            CModelFlags::AlphaBlended(ballAlpha * drawAlpha).UseShaderSet(x1e0_ballMatIdx)};
        x184_ballModelData->MultipassDraw(CModelData::kWM_Normal, x10_ballTransform,
                                          x24c_actorLights.get(), flags, 2);
        x184_ballModelData->Render(
            mgr, x10_ballTransform, nullptr,
            CModelFlags::Additive(x6c_ballPulseFactor * (itemPulse * (ballAlpha * drawAlpha)))
                .UseShaderSet(x1e0_ballMatIdx));
      }
      if (x4d_selectedMorphball && ballT > 0.5f) {
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
          x184_ballModelData->Render(mgr,
                                     x10_ballTransform * CTransform4f::RotateZ(spinAngle) *
                                         CTransform4f::Scale(spinScale, spinScale, spinScale),
                                     x24c_actorLights.get(),
                                     CModelFlags::Additive(spinAlpha * drawAlpha)
                                         .DepthCompareUpdate(true, false)
                                         .UseShaderSet(x1e0_ballMatIdx));
        }
      }
      if (x270_24_hasSpiderBall && ballAlpha > 0.f) {
        CGraphics::SetModelMatrix(x10_ballTransform);
        glass->Draw(CModelFlags::AlphaBlended(0.f)
                        .DepthCompareUpdate(true, true)
                        .UseShaderSet(x1e4_glassMatIdx));
        glass->Draw(
            CModelFlags::AlphaBlended(ballAlpha * drawAlpha).UseShaderSet(x1e4_glassMatIdx));
        glass->Draw(
            CModelFlags::Additive(x6c_ballPulseFactor * (itemPulse * (ballAlpha * drawAlpha)))
                .UseShaderSet(x1e4_glassMatIdx));
      }
    }
    if (phazonSuit && drawAlpha > 0.1f) {
      CCubeRenderer* const renderer = gpRender;
      const float radius = CMath::Clamp(
          0.2f, (10.f - (xc0_userZoom >= 0.f ? xc0_userZoom : -xc0_userZoom)) / 20.f, 1.f);
      renderer->DrawPhazonSuitIndirectEffect(
          CColor(0.1f, 0.1f, 0.1f, drawAlpha), x250_phazonIndirectTexture,
          radius, 0.1f, CMath::FastSinR(x260_phazonOffsetAngle),
          CMath::FastSinR(x260_phazonOffsetAngle),
          gpTweakGuiColors->GetPauseBlurFilterColor().WithAlphaOf(drawAlpha));
    }
  } else {
    CModelFlags flags[2] = {
        CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, true).UseShaderSet(x1e0_ballMatIdx),
        CModelFlags::AlphaBlended(drawAlpha).UseShaderSet(x1e0_ballMatIdx)};
    x184_ballModelData->MultipassDraw(CModelData::kWM_Normal, x10_ballTransform,
                                      x24c_actorLights.get(), flags, 2);
    x184_ballModelData->Render(mgr, x10_ballTransform, nullptr,
                               CModelFlags::Additive(x6c_ballPulseFactor * (drawAlpha * itemPulse))
                                   .UseShaderSet(x1e0_ballMatIdx));
    const CMorphBall::SColorRgb& glow = CMorphBall::skBallInnerGlowColors[x1e8_ballGlowColorIdx];
    x22c_ballInnerGlowGen->SetModulationColor(
        CColor(glow.x0_r, glow.x1_g, glow.x2_b).WithAlphaOf(drawAlpha));
    if (drawAlpha > 0.f) {
      if (x22c_ballInnerGlowGen->GetNumActiveChildParticles() > 0) {
        CParticleGen* child = x22c_ballInnerGlowGen->GetActiveChildParticle(0);
        const CMorphBall::SColorRgb& hull = CMorphBall::skBallHullGlowColors[x1e8_ballGlowColorIdx];
        child->SetModulationColor(CColor(hull.x0_r, hull.x1_g, hull.x2_b).WithAlphaOf(drawAlpha));
        if (x22c_ballInnerGlowGen->GetNumActiveChildParticles() > 1) {
          CParticleGen* child = x22c_ballInnerGlowGen->GetActiveChildParticle(1);
          const CMorphBall::SColorRgb& aux =
              CMorphBall::skBallBoostedHullGlowColors[x1e8_ballGlowColorIdx];
          child->SetModulationColor(CColor(aux.x0_r, aux.x1_g, aux.x2_b).WithAlphaOf(drawAlpha));
        }
      }
      x22c_ballInnerGlowGen->Render();
    }
    if (x270_24_hasSpiderBall) {
      CGraphics::SetModelMatrix(x10_ballTransform);
      glass->Draw(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, true));
      glass->Draw(CModelFlags::AlphaBlended(drawAlpha).UseShaderSet(x1e4_glassMatIdx));
      glass->Draw(CModelFlags::Additive(x6c_ballPulseFactor * (drawAlpha * itemPulse))
                      .UseShaderSet(x1e4_glassMatIdx));
    }
  }
  if (!x238_ballTransitionFlashGen.null()) {
    const CMorphBall::SColorRgb& color = CMorphBall::skBallHullGlowColors[x1e8_ballGlowColorIdx];
    x238_ballTransitionFlashGen->SetModulationColor(CColor(color.x0_r, color.x1_g, color.x2_b));
    x238_ballTransitionFlashGen->Render();
  }
  CGraphics::DisableAllLights();
}

void CSamusDoll::CheckTransition(bool morphball) {
  if (x54_remTransitionTime > 0.f || morphball == x4d_selectedMorphball) {
    return;
  }
  x4d_selectedMorphball = morphball;
  SetTransitionAnimation();
}

void CSamusDoll::SetTransitionAnimation() {
  if (!x4c_completedMorphball) {
    xc8_suitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
    x134_suitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
    x54_remTransitionTime = x50_totalTransitionTime =
        xc8_suitModel0->AnimationData()->GetAnimationDuration(0);
  } else if (!x4d_selectedMorphball) {
    xc8_suitModel0->AnimationData()->SetAnimation(CAnimPlaybackParms(1, -1, 1.f, true), false);
    x134_suitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms(1, -1, 1.f, true), false);
    x54_remTransitionTime = x50_totalTransitionTime =
        xc8_suitModel0->AnimationData()->GetAnimationDuration(1);
  }
}

void CSamusDoll::SetPulseBeam(bool pulse) { x270_27_pulseBeam = pulse; }
void CSamusDoll::SetPulseGrapple(bool pulse) { x270_28_pulseGrapple = pulse; }
void CSamusDoll::SetPulseBoots(bool pulse) { x270_29_pulseBoots = pulse; }
void CSamusDoll::SetPulseVisor(bool pulse) { x270_30_pulseVisor = pulse; }
void CSamusDoll::SetPulseSuit(bool pulse) { x270_26_pulseSuit = pulse; }

void CSamusDoll::SetRotation(float xDelta, float zDelta, float dt) {
  if (IsViewInterpolating()) {
    return;
  }
  SetRotationSfxPlaying(xDelta != 0.f || zDelta != 0.f);
  CEulerAngles angles = CEulerAngles::FromQuaternion(xb0_userRot);
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
  xb0_userRot =
      CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(clampedX),
                               CRelAngle::FromRadians(angles.GetZ()));
}

void CSamusDoll::SetOffset(const CVector3f& offset, float dt) {
  if (IsViewInterpolating()) {
    return;
  }
  CVector3f oldOffset = xa4_offset;
  const CVector3f move(offset.GetX(), 0.f, offset.GetZ());
  const CMatrix3f rotation = xb0_userRot.BuildTransform();
  const CVector3f rotatedMove = rotation * move;
  xa4_offset += rotatedMove;
  if ((oldOffset - xa4_offset).Magnitude() > dt) {
    SetOffsetSfxPlaying(true);
  } else {
    SetOffsetSfxPlaying(false);
  }
  const float oldZoom = xc0_userZoom;
  static const float skMinZoom = -4.f;
  static const float skMaxZoom = -2.2f;
  xc0_userZoom = CMath::Clamp(skMinZoom, xc0_userZoom + offset.GetY(), skMaxZoom);
  bool zoomSfx = false;
  if (CMath::AbsF(xc0_userZoom - oldZoom) > dt) {
    zoomSfx = true;
  }
  const float zoomDelta = offset.GetY() - (xc0_userZoom - oldZoom);
  const CVector3f newOffset = xa4_offset + rotation.GetColumn(kDY) * zoomDelta;
  CVector3f delta = newOffset - xa4_offset;
  const CVector3f beforeZoomOffset = xa4_offset;
  if (!(delta == CVector3f::Zero())) {
    float time;
    CVector3f point = CVector3f::Zero();
    if (CollisionUtil::RaySphereIntersection(CSphere(skInitialOffset, 1.f), xa4_offset,
                                             delta.AsNormalized(), 0.f, time, point)) {
      const float pointDistance = (point - xa4_offset).Magnitude();
      if (pointDistance < CMath::AbsF(zoomDelta)) {
        xa4_offset = point;
      } else {
        xa4_offset = newOffset;
      }
    } else {
      xa4_offset = newOffset;
    }
  }
  if ((beforeZoomOffset - xa4_offset).Magnitude() > dt) {
    zoomSfx = true;
  }
  SetZoomSfxPlaying(zoomSfx);
  const CVector3f offsetDelta = xa4_offset - skInitialOffset;
  if (offsetDelta.Magnitude() > 1.f) {
    xa4_offset = skInitialOffset + 1.f * offsetDelta.AsNormalized();
  }
}

void CSamusDoll::BeginViewInterpolate(bool zoomIn) {
  if (xc4_viewInterp == 0.f) {
    CSfxManager::SfxStart(0x5a0);
  } else if (xc4_viewInterp == 1.f) {
    SetOffsetSfxPlaying(false);
    SetZoomSfxPlaying(false);
    SetRotationSfxPlaying(false);
    CSfxManager::SfxStart(0x5a1);
  } else {
    return;
  }
  xc4_viewInterp = zoomIn ? FLT_EPSILON : -1.f + FLT_EPSILON;
  x84_interpStartOffset = xa4_offset;
  x90_userInterpRot = xb0_userRot;
  xa0_userInterpZoom = xc0_userZoom;
  x80_fixedZoom = zoomIn ? -2.2f : -3.6f;
}

void CSamusDoll::SetOffsetSfxPlaying(bool playing) {
  if (playing) {
    if (!x264_offsetSfx) {
      x264_offsetSfx =
          CSfxManager::SfxStart(0x57e, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(x264_offsetSfx);
    x264_offsetSfx.Clear();
  }
}

void CSamusDoll::SetZoomSfxPlaying(bool playing) {
  if (playing) {
    if (!x26c_zoomSfx) {
      x26c_zoomSfx = CSfxManager::SfxStart(0x560, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(x26c_zoomSfx);
    x26c_zoomSfx.Clear();
  }
}

void CSamusDoll::SetRotationSfxPlaying(bool playing) {
  if (playing) {
    if (!x268_rotateSfx) {
      x268_rotateSfx =
          CSfxManager::SfxStart(0x55f, 127, 64, false, CSfxManager::kMedPriority, true);
    }
  } else {
    CSfxManager::SfxStop(x268_rotateSfx);
    x268_rotateSfx.Clear();
  }
}
