#include "Runtime/MP1/CSamusDoll.hpp"

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CollisionUtil.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CMorphBall.hpp"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <memory>

#include <zeus/CColor.hpp>
#include <zeus/CEulerAngles.hpp>

namespace urde::MP1 {
namespace {
constexpr std::array<std::pair<const char*, u32>, 8> SpiderBallGlassModels{{
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
}};

constexpr std::array<std::pair<const char*, u32>, 8> SpiderBallCharacters{{
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 1},
    {"SamusPhazonBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
}};

constexpr std::array<std::pair<const char*, u32>, 8> BallCharacters{{
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 1},
    {"SamusBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
}};

constexpr std::array<u32, 8> SpiderBallGlowColorIdxs{
    3, 3, 2, 4, 5, 7, 6, 8,
};

constexpr std::array<u32, 8> BallGlowColorIdxs{
    0, 0, 1, 0, 5, 7, 6, 8,
};

constexpr std::array BeamModels{
    "CMDL_InvPowerBeam", "CMDL_InvIceBeam", "CMDL_InvWaveBeam", "CMDL_InvPlasmaBeam", "CMDL_InvPowerBeam",
};

constexpr std::array VisorModels{
    "CMDL_InvVisor",       "CMDL_InvGravityVisor", "CMDL_InvVisor",       "CMDL_InvPhazonVisor",
    "CMDL_InvFusionVisor", "CMDL_InvFusionVisor",  "CMDL_InvFusionVisor", "CMDL_InvFusionVisor",
};

constexpr std::array FinModels{
    "CMDL_InvPowerFins", "CMDL_InvPowerFins", "CMDL_InvPowerFins",   "CMDL_InvPowerFins",
    "CMDL_InvPowerFins", "CMDL_InvVariaFins", "CMDL_InvGravityFins", "CMDL_InvPhazonFins",
};

constexpr std::array<u32, 8> Character1Idxs{
    0, 6, 2, 10, 16, 24, 20, 28,
};

constexpr std::array<u32, 8> CharacterBootsIdxs{
    1, 7, 3, 11, 17, 25, 21, 29,
};

constexpr std::array<std::array<u32, 2>, 8> Character2and3Idxs{{
    {14, 15},
    {8, 9},
    {4, 5},
    {12, 13},
    {18, 19},
    {26, 27},
    {22, 23},
    {30, 31},
}};
} // Anonymous namespace

CSamusDoll::CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
                       CPlayerState::EPlayerSuit suit, CPlayerState::EBeamId beam, bool hasSpiderBall,
                       bool hasGrappleBeam)
: x10_ballXf(zeus::CTransform::Translate(0.f, 0.f, 0.625f * g_tweakPlayer->GetPlayerBallHalfExtent()))
, x44_suit(suit)
, x48_beam(beam)
, x270_24_hasSpiderBall(hasSpiderBall)
, x270_25_hasGrappleBeam(hasGrappleBeam) {
  x70_fixedRot.rotateZ(M_PIF);
  x90_userInterpRot = xb0_userRot = x70_fixedRot;
  x1d4_spiderBallGlass = g_SimplePool->GetObj(SpiderBallGlassModels[size_t(suit)].first);
  x1e0_ballMatIdx = hasSpiderBall ? SpiderBallCharacters[size_t(suit)].second : BallCharacters[size_t(suit)].second;
  x1e4_glassMatIdx = SpiderBallGlassModels[size_t(suit)].second;
  x1e8_ballGlowColorIdx = hasSpiderBall ? SpiderBallGlowColorIdxs[size_t(suit)] : BallGlowColorIdxs[size_t(suit)];
  x1ec_itemScreenSamus = g_SimplePool->GetObj("ANCS_ItemScreenSamus");
  x1f4_invBeam = g_SimplePool->GetObj(BeamModels[size_t(beam)]);
  x200_invVisor = g_SimplePool->GetObj(VisorModels[size_t(suit)]);
  x20c_invGrappleBeam = g_SimplePool->GetObj("CMDL_InvGrappleBeam");
  x218_invFins = g_SimplePool->GetObj(FinModels[size_t(suit)]);
  x224_ballInnerGlow = g_SimplePool->GetObj("BallInnerGlow");
  x22c_ballInnerGlowGen = std::make_unique<CElementGen>(x224_ballInnerGlow);
  x230_ballTransitionFlash = g_SimplePool->GetObj("MorphBallTransitionFlash");
  x23c_lights.push_back(CLight::BuildDirectional(zeus::skForward, zeus::skWhite));
  x24c_actorLights = std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
  x22c_ballInnerGlowGen->SetGlobalScale(zeus::CVector3f(0.625f));
  x0_depToks.reserve(suitDgrp.GetObjectTagVector().size() + ballDgrp.GetObjectTagVector().size());
  for (const SObjectTag& tag : suitDgrp.GetObjectTagVector()) {
    x0_depToks.push_back(g_SimplePool->GetObj(tag));
    x0_depToks.back().Lock();
  }
  for (const SObjectTag& tag : ballDgrp.GetObjectTagVector()) {
    x0_depToks.push_back(g_SimplePool->GetObj(tag));
    x0_depToks.back().Lock();
  }
}

bool CSamusDoll::IsLoaded() const {
  if (x270_31_loaded)
    return true;
  if (!x1ec_itemScreenSamus.IsLoaded())
    return false;
  if (!x1f4_invBeam.IsLoaded())
    return false;
  if (!x200_invVisor.IsLoaded())
    return false;
  if (!x20c_invGrappleBeam.IsLoaded())
    return false;
  if (!x1d4_spiderBallGlass.IsLoaded())
    return false;
  if (x218_invFins && !x218_invFins.IsLoaded())
    return false;
  return xc8_suitModel0.operator bool();
}

CModelData CSamusDoll::BuildSuitModelData1(CPlayerState::EPlayerSuit suit) {
  CModelData ret(CAnimRes(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id, Character1Idxs[size_t(suit)],
                          zeus::skOne3f, 2, true));
  constexpr CAnimPlaybackParms parms(2, -1, 1.f, true);
  ret.GetAnimationData()->SetAnimation(parms, false);
  return ret;
}

CModelData CSamusDoll::BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit) {
  CModelData ret(CAnimRes(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id,
                          CharacterBootsIdxs[size_t(suit)], zeus::skOne3f, 2, true));
  constexpr CAnimPlaybackParms parms(2, -1, 1.f, true);
  ret.GetAnimationData()->SetAnimation(parms, false);
  return ret;
}

bool CSamusDoll::CheckLoadComplete() {
  if (IsLoaded())
    return true;

  for (const CToken& tok : x0_depToks)
    if (!tok.IsLoaded())
      return false;

  xc8_suitModel0.emplace(BuildSuitModelData1(x44_suit));
  for (int i = 0; i < 2; ++i) {
    CAnimRes res(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id, Character2and3Idxs[size_t(x44_suit)][i],
                 zeus::skOne3f, 2, true);
    CModelData mData(res);
    x118_suitModel1and2.push_back(mData.GetAnimationData()->GetModelData());
    x118_suitModel1and2.back().Lock();
  }
  x134_suitModelBoots.emplace(BuildSuitModelDataBoots(x44_suit));

  CAnimRes res(g_ResFactory
                   ->GetResourceIdByName(x270_24_hasSpiderBall ? SpiderBallCharacters[size_t(x44_suit)].first
                                                               : BallCharacters[size_t(x44_suit)].first)
                   ->id,
               0, zeus::skOne3f, 0, true);
  x184_ballModelData.emplace(res);
  x1e0_ballMatIdx =
      x270_24_hasSpiderBall ? SpiderBallCharacters[size_t(x44_suit)].second : BallCharacters[size_t(x44_suit)].second;
  x270_31_loaded = true;
  return true;
}

void CSamusDoll::Update(float dt, CRandom16& rand) {
  if (x1f4_invBeam.IsLoaded())
    x1f4_invBeam->Touch(0);
  if (x200_invVisor.IsLoaded())
    x200_invVisor->Touch(0);
  if (x20c_invGrappleBeam.IsLoaded())
    x20c_invGrappleBeam->Touch(0);
  if (x1d4_spiderBallGlass.IsLoaded())
    x1d4_spiderBallGlass->Touch(0);
  if (x218_invFins.IsLoaded())
    x218_invFins->Touch(0);

  if (!CheckLoadComplete())
    return;

  x40_alphaIn = std::min(x40_alphaIn + 2.f * dt, 1.f);
  if (x54_remTransitionTime > 0.f) {
    float oldRemTransTime = x54_remTransitionTime;
    x54_remTransitionTime = std::max(0.f, x54_remTransitionTime - dt);
    if (!x4c_completedMorphball && x4d_selectedMorphball && oldRemTransTime >= x50_totalTransitionTime - 0.5f &&
        x54_remTransitionTime < x50_totalTransitionTime - 0.5f) {
      x238_ballTransitionFlashGen = std::make_unique<CElementGen>(x230_ballTransitionFlash);
      x238_ballTransitionFlashGen->SetGlobalScale(zeus::CVector3f(0.625f));
    }

    if (x54_remTransitionTime == 0.f) {
      x4c_completedMorphball = x4d_selectedMorphball;
      if (!x4d_selectedMorphball) {
        xc8_suitModel0->GetAnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
        x134_suitModelBoots->GetAnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
      }
    }
  }

  if (x270_26_pulseSuit)
    x58_suitPulseFactor = std::min(x58_suitPulseFactor + 2.f * dt, 1.f);
  else
    x58_suitPulseFactor = std::max(x58_suitPulseFactor - 2.f * dt, 0.f);

  if (x270_27_pulseBeam)
    x5c_beamPulseFactor = std::min(x5c_beamPulseFactor + 2.f * dt, 1.f);
  else
    x5c_beamPulseFactor = std::max(x5c_beamPulseFactor - 2.f * dt, 0.f);

  if (x270_28_pulseGrapple)
    x60_grapplePulseFactor = std::min(x60_grapplePulseFactor + 2.f * dt, 1.f);
  else
    x60_grapplePulseFactor = std::max(x60_grapplePulseFactor - 2.f * dt, 0.f);

  if (x270_29_pulseBoots)
    x64_bootsPulseFactor = std::min(x64_bootsPulseFactor + 2.f * dt, 1.f);
  else
    x64_bootsPulseFactor = std::max(x64_bootsPulseFactor - 2.f * dt, 0.f);

  if (x270_30_pulseVisor)
    x68_visorPulseFactor = std::min(x68_visorPulseFactor + 2.f * dt, 1.f);
  else
    x68_visorPulseFactor = std::max(x68_visorPulseFactor - 2.f * dt, 0.f);

  if (x4c_completedMorphball)
    x6c_ballPulseFactor = std::min(x6c_ballPulseFactor + 2.f * dt, 1.f);
  else
    x6c_ballPulseFactor = std::max(x6c_ballPulseFactor - 2.f * dt, 0.f);

  if (x44_suit == CPlayerState::EPlayerSuit::Phazon) {
    if (!x250_phazonIndirectTexture)
      x250_phazonIndirectTexture = g_SimplePool->GetObj("PhazonIndirectTexture");
  } else {
    if (x250_phazonIndirectTexture)
      x250_phazonIndirectTexture = TLockedToken<CTexture>();
  }

  if (x250_phazonIndirectTexture) {
    x260_phazonOffsetAngle += 0.03f;
    x260_phazonOffsetAngle.makeRel();
    g_Renderer->AllocatePhazonSuitMaskTexture();
  }

  xc8_suitModel0->AdvanceAnimationIgnoreParticles(dt, rand, true);
  x134_suitModelBoots->AdvanceAnimationIgnoreParticles(dt, rand, true);
  x184_ballModelData->AdvanceAnimationIgnoreParticles(dt, rand, true);

  SetupLights();

  x22c_ballInnerGlowGen->SetGlobalTranslation(x10_ballXf.origin);
  x22c_ballInnerGlowGen->Update(dt);

  if (x238_ballTransitionFlashGen) {
    if (x238_ballTransitionFlashGen->IsSystemDeletable())
      x238_ballTransitionFlashGen.reset();
    if (x238_ballTransitionFlashGen) {
      x22c_ballInnerGlowGen->SetGlobalTranslation(x10_ballXf.origin);
      x22c_ballInnerGlowGen->Update(dt);
    }
  }

  if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f) {
    if (xc4_viewInterp < 0.f)
      xc4_viewInterp = std::min(xc4_viewInterp + 3.f * dt, 0.f);
    else
      xc4_viewInterp = std::min(xc4_viewInterp + 3.f * dt, 1.f);

    float interp = std::fabs(xc4_viewInterp);
    float oneMinusInterp = 1.f - interp;
    xa4_offset = x84_interpStartOffset * interp + skInitialOffset * oneMinusInterp;

    xb0_userRot = zeus::CQuaternion::slerpShort(x70_fixedRot, x90_userInterpRot, interp);

    if (xc4_viewInterp <= 0.f) // Zoom out
      xc0_userZoom = x80_fixedZoom * oneMinusInterp + xa0_userInterpZoom * interp;
    else // Zoom in
      xc0_userZoom = x80_fixedZoom * interp + xa0_userInterpZoom * oneMinusInterp;
  }
}

void CSamusDoll::Draw(const CStateManager& mgr, float alpha) {
  if (!IsLoaded())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CSamusDoll::Draw", zeus::skPurple);

  alpha *= x40_alphaIn;

  float itemPulse = zeus::clamp(0.f, (std::sin(5.f * CGraphics::GetSecondsMod900()) + 1.f) * 0.5f, 1.f) *
                    (1.f - std::fabs(xc4_viewInterp));

  g_Renderer->SetPerspective(55.f, g_Viewport.x8_width, g_Viewport.xc_height, 0.2f, 4096.f);

  CGraphics::SetViewPointMatrix(zeus::CTransform(xb0_userRot, xa4_offset) *
                                zeus::CTransform::Translate(0.f, xc0_userZoom, 0.f));

  zeus::CTransform gunXf = xc8_suitModel0->GetScaledLocatorTransform("GUN_LCTR");
  zeus::CTransform visorXf = xc8_suitModel0->GetScaledLocatorTransform("VISOR_LCTR");
  zeus::CTransform grappleXf = xc8_suitModel0->GetScaledLocatorTransform("GRAPPLE_LCTR");

  if (!x4c_completedMorphball || !x4d_selectedMorphball) {
    float suitPulse = itemPulse * x58_suitPulseFactor;
    float bootsPulse = std::max(suitPulse, itemPulse * x64_bootsPulseFactor);

    bool phazonSuit = x44_suit == CPlayerState::EPlayerSuit::Phazon;
    // Enable dst alpha 1.0

    for (size_t i = 0; i <= x118_suitModel1and2.size(); ++i) {
      TCachedToken<CSkinnedModel> backupModelData = xc8_suitModel0->GetAnimationData()->GetModelData();
      if (i < x118_suitModel1and2.size())
        xc8_suitModel0->GetAnimationData()->SubstituteModelData(x118_suitModel1and2[i]);
      xc8_suitModel0->InvSuitDraw(CModelData::EWhichModel::Normal, zeus::CTransform(), x24c_actorLights.get(),
                                  zeus::CColor(1.f, alpha), zeus::CColor(1.f, alpha * suitPulse));
      xc8_suitModel0->GetAnimationData()->SubstituteModelData(backupModelData);
    }

    x134_suitModelBoots->InvSuitDraw(CModelData::EWhichModel::Normal, zeus::CTransform(),
                                     x24c_actorLights.get(), zeus::CColor(1.f, alpha),
                                     zeus::CColor(1.f, alpha * bootsPulse));

    {
      CGraphics::SetModelMatrix(gunXf);
      x1f4_invBeam->GetInstance().ActivateLights(x23c_lights);
      CModelFlags flags = {};

      flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
      flags.x4_color = zeus::skWhite;
      x1f4_invBeam->Draw(flags);

      flags.m_extendedShader = EExtendedShader::ForcedAlpha;
      flags.x4_color = zeus::CColor(1.f, alpha);
      x1f4_invBeam->Draw(flags);

      flags.m_extendedShader = EExtendedShader::ForcedAdditive;
      flags.x4_color = zeus::CColor(1.f, alpha * itemPulse * x5c_beamPulseFactor);
      x1f4_invBeam->Draw(flags);
    }

    {
      CGraphics::SetModelMatrix(visorXf);

      float visorT = std::fmod(CGraphics::GetSecondsMod900(), 1.f) * (1.f - std::fabs(xc4_viewInterp));
      float alphaBlend = (visorT < 0.25f) ? 1.f - 2.f * visorT : (visorT < 0.5f) ? 2.f * (visorT - 0.25f) + 0.5f : 1.f;
      float addBlend = (visorT > 0.75f) ? 1.f - 4.f * (visorT - 0.75f) : (visorT > 0.5f) ? 4.f * (visorT - 0.5f) : 0.f;

      x200_invVisor->GetInstance().ActivateLights(x23c_lights);
      CModelFlags flags = {};
      flags.m_extendedShader = EExtendedShader::Lighting;
      flags.x4_color =
          zeus::CColor::lerp(zeus::CColor(1.f, alpha), zeus::CColor(alphaBlend, alpha), x68_visorPulseFactor);
      x200_invVisor->Draw(flags);

      flags.m_extendedShader = EExtendedShader::ForcedAdditive;
      flags.x4_color = zeus::CColor(1.f, alpha * addBlend * x68_visorPulseFactor);
      x200_invVisor->Draw(flags);
    }

    if (x270_25_hasGrappleBeam) {
      CGraphics::SetModelMatrix(grappleXf);

      x20c_invGrappleBeam->GetInstance().ActivateLights(x23c_lights);
      CModelFlags flags = {};
      flags.m_extendedShader = EExtendedShader::ForcedAlpha;
      flags.x4_color = zeus::CColor(1.f, alpha);
      x20c_invGrappleBeam->Draw(flags);

      flags.m_extendedShader = EExtendedShader::ForcedAdditive;
      flags.x4_color = zeus::CColor(1.f, alpha * itemPulse * x60_grapplePulseFactor);
      x20c_invGrappleBeam->Draw(flags);
    } else if (x44_suit >= CPlayerState::EPlayerSuit::FusionPower) {
      CGraphics::SetModelMatrix(grappleXf);

      x218_invFins->GetInstance().ActivateLights(x23c_lights);
      CModelFlags flags = {};
      flags.m_extendedShader = EExtendedShader::ForcedAlpha;
      flags.x4_color = zeus::CColor(1.f, alpha);
      x218_invFins->Draw(flags);

      flags.m_extendedShader = EExtendedShader::ForcedAdditive;
      flags.x4_color = zeus::CColor(1.f, alpha * suitPulse);
      x218_invFins->Draw(flags);
    }

    if (x54_remTransitionTime > 0.f) {
      float ballT = 1.f - x54_remTransitionTime / x50_totalTransitionTime;

      float ballAlpha = 0.f;
      if (x4d_selectedMorphball)
        ballAlpha = 1.f - std::min(x54_remTransitionTime / 0.25f, 1.f);
      else if (x4c_completedMorphball)
        ballAlpha = std::max(0.f, (x54_remTransitionTime - (x50_totalTransitionTime - 0.25f)) / 0.25f);

      if (ballAlpha > 0.f) {
        CModelFlags flags = {};
        flags.x1_matSetIdx = x1e0_ballMatIdx;
        flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
        flags.x4_color = zeus::skWhite;
        x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);

        flags.m_extendedShader = EExtendedShader::ForcedAlpha;
        flags.x4_color = zeus::skWhite;
        flags.x4_color.a() = alpha * ballAlpha;
        x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);

        flags.m_extendedShader = EExtendedShader::ForcedAdditive;
        flags.x4_color = zeus::skWhite;
        flags.x4_color.a() = x6c_ballPulseFactor * alpha * ballAlpha * itemPulse;
        x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);
      }

      if (x4d_selectedMorphball && ballT > 0.5f) {
        float ballEndT = (ballT - 0.5f) / 0.5f;
        float oneMinusBallEndT = 1.f - ballEndT;

        float spinScale = 0.75f * oneMinusBallEndT + 1.f;
        float spinAlpha;
        if (ballEndT < 0.1f)
          spinAlpha = 0.f;
        else if (ballEndT < 0.2f)
          spinAlpha = (ballEndT - 0.1f) / 0.1f;
        else if (ballEndT < 0.9f)
          spinAlpha = 1.f;
        else
          spinAlpha = 1.f - (ballT - 0.9f) / 0.1f;

        zeus::CRelAngle spinAngle = zeus::degToRad(360.f * oneMinusBallEndT);
        spinAlpha *= 0.5f;
        if (spinAlpha > 0.f) {
          CModelFlags flags = {};
          flags.m_extendedShader = EExtendedShader::ForcedAdditive;
          flags.x1_matSetIdx = x1e0_ballMatIdx;
          flags.x4_color = zeus::CColor(1.f, spinAlpha * alpha);
          x184_ballModelData->Render(
              mgr, x10_ballXf * zeus::CTransform::RotateZ(spinAngle) * zeus::CTransform::Scale(spinScale),
              x24c_actorLights.get(), flags);
        }
      }

      if (x270_24_hasSpiderBall) {
        CGraphics::SetModelMatrix(x10_ballXf);
        CModelFlags flags = {};
        flags.x1_matSetIdx = x1e4_glassMatIdx;
        x1d4_spiderBallGlass->GetInstance().ActivateLights(x23c_lights);

        flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
        flags.x4_color = zeus::skWhite;
        x1d4_spiderBallGlass->Draw(flags);

        flags.m_extendedShader = EExtendedShader::ForcedAlpha;
        flags.x4_color = zeus::skWhite;
        flags.x4_color.a() = alpha;
        x1d4_spiderBallGlass->Draw(flags);

        flags.m_extendedShader = EExtendedShader::ForcedAdditive;
        flags.x4_color = zeus::skWhite;
        flags.x4_color.a() = x6c_ballPulseFactor * alpha * itemPulse;
        x1d4_spiderBallGlass->Draw(flags);
      }
    }

    if (phazonSuit && alpha > 0.1f) {
      float radius = zeus::clamp(0.2f, (10.f - (xc0_userZoom >= 0.f ? xc0_userZoom : -xc0_userZoom)) / 20.f, 1.f);
      float offset = std::sin(x260_phazonOffsetAngle);
      zeus::CColor color = g_tweakGuiColors->GetPauseBlurFilterColor();
      color.a() = alpha;
      g_Renderer->DrawPhazonSuitIndirectEffect(zeus::CColor(0.1f, alpha), x250_phazonIndirectTexture, color, radius,
                                               0.1f, offset, offset);
    }
  } else {
    CModelFlags flags = {};
    flags.x1_matSetIdx = x1e0_ballMatIdx;
    flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
    flags.x4_color = zeus::skWhite;
    x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);

    flags.m_extendedShader = EExtendedShader::ForcedAlpha;
    flags.x4_color = zeus::skWhite;
    flags.x4_color.a() = alpha;
    x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);

    flags.m_extendedShader = EExtendedShader::ForcedAdditive;
    flags.x4_color = zeus::skWhite;
    flags.x4_color.a() = x6c_ballPulseFactor * alpha * itemPulse;
    x184_ballModelData->Render(mgr, x10_ballXf, x24c_actorLights.get(), flags);

    const CMorphBall::ColorArray ballGlowColorData = CMorphBall::BallGlowColors[x1e8_ballGlowColorIdx];
    const zeus::CColor ballGlowColor{
        float(ballGlowColorData[0]) / 255.f,
        float(ballGlowColorData[1]) / 255.f,
        float(ballGlowColorData[2]) / 255.f,
        alpha,
    };
    x22c_ballInnerGlowGen->SetModulationColor(ballGlowColor);

    if (alpha > 0.f) {
      if (x22c_ballInnerGlowGen->GetNumActiveChildParticles() > 0) {
        const CMorphBall::ColorArray transFlashColorData = CMorphBall::BallTransFlashColors[x1e8_ballGlowColorIdx];
        const zeus::CColor transFlashColor{
            float(transFlashColorData[0]) / 255.f,
            float(transFlashColorData[1]) / 255.f,
            float(transFlashColorData[2]) / 255.f,
            alpha,
        };
        x22c_ballInnerGlowGen->GetActiveChildParticle(0).SetModulationColor(transFlashColor);

        if (x22c_ballInnerGlowGen->GetNumActiveChildParticles() > 1) {
          const CMorphBall::ColorArray auxColorData = CMorphBall::BallAuxGlowColors[x1e8_ballGlowColorIdx];
          const zeus::CColor auxColor{
              float(auxColorData[0]) / 255.f,
              float(auxColorData[1]) / 255.f,
              float(auxColorData[2]) / 255.f,
              alpha,
          };
          x22c_ballInnerGlowGen->GetActiveChildParticle(1).SetModulationColor(auxColor);
        }
      }
      x22c_ballInnerGlowGen->Render();
    }

    if (x270_24_hasSpiderBall) {
      CGraphics::SetModelMatrix(x10_ballXf);
      CModelFlags spiderBallGlassFlags = {};
      spiderBallGlassFlags.x1_matSetIdx = x1e4_glassMatIdx;
      x1d4_spiderBallGlass->GetInstance().ActivateLights(x23c_lights);

      spiderBallGlassFlags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
      spiderBallGlassFlags.x4_color = zeus::skWhite;
      x1d4_spiderBallGlass->Draw(spiderBallGlassFlags);

      spiderBallGlassFlags.m_extendedShader = EExtendedShader::ForcedAlpha;
      spiderBallGlassFlags.x4_color = zeus::skWhite;
      spiderBallGlassFlags.x4_color.a() = alpha;
      x1d4_spiderBallGlass->Draw(spiderBallGlassFlags);

      spiderBallGlassFlags.m_extendedShader = EExtendedShader::ForcedAdditive;
      spiderBallGlassFlags.x4_color = zeus::skWhite;
      spiderBallGlassFlags.x4_color.a() = x6c_ballPulseFactor * alpha * itemPulse;
      x1d4_spiderBallGlass->Draw(spiderBallGlassFlags);
    }
  }

  if (x238_ballTransitionFlashGen) {
    const CMorphBall::ColorArray c = CMorphBall::BallTransFlashColors[x1e8_ballGlowColorIdx];
    const zeus::CColor color{
        float(c[0]) / 255.f,
        float(c[1]) / 255.f,
        float(c[2]) / 255.f,
        1.f,
    };
    x238_ballTransitionFlashGen->SetModulationColor(color);
    x238_ballTransitionFlashGen->Render();
  }

  CGraphics::DisableAllLights();
}

void CSamusDoll::Touch() {
  if (!CheckLoadComplete())
    return;
  xc8_suitModel0->GetAnimationData()->PreRender();
  x134_suitModelBoots->GetAnimationData()->PreRender();
  x184_ballModelData->GetAnimationData()->PreRender();
  xc8_suitModel0->Touch(CModelData::EWhichModel::Normal, 0);
  x134_suitModelBoots->Touch(CModelData::EWhichModel::Normal, 0);
  x184_ballModelData->Touch(CModelData::EWhichModel::Normal, 0);
}

void CSamusDoll::SetupLights() {
  x23c_lights[0] = CLight::BuildDirectional(xb0_userRot.toTransform().basis[1], zeus::CColor(0.75f, 1.f));
  x24c_actorLights->BuildFakeLightList(x23c_lights, zeus::skBlack);
}

void CSamusDoll::SetInMorphball(bool morphball) {
  if (x54_remTransitionTime > 0.f)
    return;
  if (x4d_selectedMorphball == morphball)
    return;
  x4d_selectedMorphball = morphball;
  SetTransitionAnimation();
}

void CSamusDoll::SetTransitionAnimation() {
  if (!x4c_completedMorphball) {
    /* Into morphball */
    xc8_suitModel0->GetAnimationData()->SetAnimation(CAnimPlaybackParms{0, -1, 1.f, true}, false);
    x134_suitModelBoots->GetAnimationData()->SetAnimation(CAnimPlaybackParms{0, -1, 1.f, true}, false);
    x50_totalTransitionTime = x54_remTransitionTime = xc8_suitModel0->GetAnimationData()->GetAnimationDuration(0);
  } else if (!x4d_selectedMorphball) {
    /* Outta morphball */
    xc8_suitModel0->GetAnimationData()->SetAnimation(CAnimPlaybackParms{1, -1, 1.f, true}, false);
    x134_suitModelBoots->GetAnimationData()->SetAnimation(CAnimPlaybackParms{1, -1, 1.f, true}, false);
    x50_totalTransitionTime = x54_remTransitionTime = xc8_suitModel0->GetAnimationData()->GetAnimationDuration(1);
  }
}

void CSamusDoll::SetRotationSfxPlaying(bool playing) {
  if (playing) {
    if (x268_rotateSfx)
      return;
    x268_rotateSfx = CSfxManager::SfxStart(SFXui_map_rotate, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x268_rotateSfx);
    x268_rotateSfx.reset();
  }
}

void CSamusDoll::SetOffsetSfxPlaying(bool playing) {
  if (playing) {
    if (x264_offsetSfx)
      return;
    x264_offsetSfx = CSfxManager::SfxStart(SFXui_map_pan, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x264_offsetSfx);
    x264_offsetSfx.reset();
  }
}

void CSamusDoll::SetZoomSfxPlaying(bool playing) {
  if (playing) {
    if (x26c_zoomSfx)
      return;
    x26c_zoomSfx = CSfxManager::SfxStart(SFXui_map_zoom, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x26c_zoomSfx);
    x26c_zoomSfx.reset();
  }
}

void CSamusDoll::SetRotation(float xDelta, float zDelta, float dt) {
  if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f)
    return;
  SetRotationSfxPlaying(xDelta != 0.f || zDelta != 0.f);
  zeus::CEulerAngles angles(xb0_userRot);

  zeus::CRelAngle angX(angles.x());
  angX.makeRel();
  zeus::CRelAngle angZ(angles.z());
  angZ.makeRel();

  angX += xDelta;
  angX.makeRel();
  angZ += zDelta;
  angZ.makeRel();

  float angXCenter = angX;
  if (angXCenter > M_PIF)
    angXCenter -= 2.f * M_PIF;
  angXCenter = zeus::clamp(-1.555f, angXCenter, 1.555f);

  zeus::CQuaternion quat;
  quat.rotateZ(angZ);
  quat.rotateX(zeus::CRelAngle(angXCenter).asRel());
  xb0_userRot = quat;
}

void CSamusDoll::SetOffset(const zeus::CVector3f& offset, float dt) {
  if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f)
    return;
  zeus::CVector3f oldOffset = xa4_offset;
  zeus::CMatrix3f rotMtx = xb0_userRot.toTransform().basis;
  xa4_offset += rotMtx * zeus::CVector3f(offset.x(), 0.f, offset.z());
  SetOffsetSfxPlaying((oldOffset - xa4_offset).magnitude() > dt);
  float oldZoom = xc0_userZoom;
  xc0_userZoom = zeus::clamp(-4.f, xc0_userZoom + offset.y(), -2.2f);
  bool zoomSfx = std::fabs(xc0_userZoom - oldZoom) > dt;
  float zoomDelta = offset.y() - (xc0_userZoom - oldZoom);
  zeus::CVector3f newOffset = rotMtx[1] * zoomDelta + xa4_offset;
  zeus::CVector3f delta = newOffset - xa4_offset;
  oldOffset = xa4_offset;
  if (!delta.isZero()) {
    zeus::CSphere sphere(skInitialOffset, 1.f);
    float T;
    zeus::CVector3f point;
    if (CollisionUtil::RaySphereIntersection(sphere, xa4_offset, delta.normalized(), 0.f, T, point)) {
      if ((point - xa4_offset).magnitude() < std::fabs(zoomDelta))
        xa4_offset = point;
      else
        xa4_offset = newOffset;
    } else {
      xa4_offset = newOffset;
    }
  }
  if ((oldOffset - xa4_offset).magnitude() > dt)
    zoomSfx = true;
  SetZoomSfxPlaying(zoomSfx);
  delta = xa4_offset - skInitialOffset;
  if (delta.magnitude() > 1.f)
    xa4_offset = delta.normalized() + skInitialOffset;
}

void CSamusDoll::BeginViewInterpolate(bool zoomIn) {
  if (xc4_viewInterp == 0.f) {
    CSfxManager::SfxStart(SFXui_samus_doll_enter, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  } else {
    SetOffsetSfxPlaying(false);
    SetZoomSfxPlaying(false);
    SetRotationSfxPlaying(false);
    CSfxManager::SfxStart(SFXui_samus_doll_exit, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }

  xc4_viewInterp = zoomIn ? FLT_EPSILON : (-1.f + FLT_EPSILON);
  x84_interpStartOffset = xa4_offset;
  x90_userInterpRot = xb0_userRot;
  xa0_userInterpZoom = xc0_userZoom;
  x80_fixedZoom = zoomIn ? -2.2f : -3.6f;
}

} // namespace urde::MP1
