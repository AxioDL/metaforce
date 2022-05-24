#include "Runtime/World/CVisorFlare.hpp"

#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGX.hpp"

namespace metaforce {

std::optional<CVisorFlare::CFlareDef> CVisorFlare::LoadFlareDef(CInputStream& in) {
  u32 propCount = in.ReadLong();
  if (propCount != 4)
    return std::nullopt;

  CAssetId txtrId = in.Get<CAssetId>();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  zeus::CColor color = in.Get<zeus::CColor>();
  if (!txtrId.IsValid())
    return std::nullopt;

  TToken<CTexture> tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});

  return CFlareDef(tex, f1, f2, color);
}

CVisorFlare::CVisorFlare(EBlendMode blendMode, bool b1, float f1, float f2, float f3, u32 w1, u32 w2,
                         std::vector<CFlareDef> flares)
: x0_blendMode(blendMode)
, x4_flareDefs(std::move(flares))
, x14_b1(b1)
, x18_f1(std::max(f1, 1.0E-4f))
, x1c_f2(f2)
, x20_f3(f3)
, x2c_w1(w1)
, x30_w2(w2) {}

void CVisorFlare::Update(float dt, const zeus::CVector3f& pos, const CActor* act, CStateManager& mgr) {
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();

  if ((visor == CPlayerState::EPlayerVisor::Combat || (x2c_w1 != 1 && visor == CPlayerState::EPlayerVisor::Thermal)) &&
      mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {

    zeus::CVector3f camPos = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
    zeus::CVector3f camDiff = pos - camPos;
    const float mag = camDiff.magnitude();
    camDiff = camDiff * (1.f / mag);
    EntityList nearVec;
    mgr.BuildNearList(nearVec, camPos, camDiff, mag,
                      CMaterialFilter::MakeInclude({EMaterialTypes::Occluder}), act);
    TUniqueId id;
    CRayCastResult result = mgr.RayWorldIntersection(
        id, camPos, camDiff, mag,
        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::SeeThrough}), nearVec);

    if (result.IsInvalid()) {
      x28_ -= dt;
    } else {
      x28_ += dt;
    }
    x28_ = zeus::clamp(0.f, x28_, x18_f1);
    x24_ = 1.f - (x28_ / x18_f1);

    const auto* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    const auto dir = (pos - curCam->GetTranslation()).normalized();
    float dot = dir.dot(curCam->GetTransform().frontVector());
    x24_ *= std::max(0.f, 1.f - (x1c_f2 * 4.f * (1.f - dot)));

    if (x2c_w1 == 2) {
      mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + x24_);
    }
  }
}

void CVisorFlare::Render(const zeus::CVector3f& pos, const CStateManager& mgr) const {
  if (zeus::close_enough(x28_, x18_f1, 1.0E-5f) ||
      mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CVisorFlare::Render", zeus::skGrey);

  u32 type;
  const auto visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor == CPlayerState::EPlayerVisor::Thermal) {
    type = x2c_w1;
  } else if (visor == CPlayerState::EPlayerVisor::XRay) {
    return;
  } else {
    type = x30_w2;
  }
  if (type != 0) {
    return;
  }

  CGraphics::DisableAllLights();
  g_Renderer->SetDepthReadWrite(false, false);
  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  zeus::CVector3f camPos = cam->GetTranslation();
  zeus::CVector3f camFront = cam->GetTransform().frontVector();
  const auto invPos = CGraphics::g_ViewMatrix.inverse() * pos;
  const auto invPos2 = CGraphics::g_ViewMatrix * zeus::CVector3f{-invPos.x(), invPos.y(), -invPos.z()};
  if (!zeus::close_enough(x24_, 0.f, 1.0E-5f)) {
    float acos = 0.f;
    if (!zeus::close_enough(x20_f3, 0.f, 1.0E-5f)) {
      zeus::CVector3f camDist{pos.x() - camPos.x(), pos.y() - camPos.y(), 0.f};
      camDist.normalize();
      zeus::CVector3f camDir{camFront.x(), camFront.y(), 0.f};
      camDir.normalize();
      acos = std::acos(camDist.dot(camDir));
      if (camDist.x() * camDir.y() - camDir.x() * camDist.y() < 0.f) {
        acos = -acos;
      }
      acos = x20_f3 * acos;
    }
    SetupRenderState(mgr);
    for (const auto& item : x4_flareDefs) {
      const auto origin = pos * (1.f - item.GetPosition()) + invPos2 * item.GetPosition();
      g_Renderer->SetModelMatrix(zeus::lookAt(origin, camPos));
      float scale = 0.5f * x24_ * item.GetScale();
      if (x14_b1) {
        auto dist = origin - camPos;
        if (dist.canBeNormalized()) {
          scale *= dist.magnitude();
        }
      }
      if (item.GetTexture()) {
        item.GetTexture()->Load(GX::TEXMAP0, EClampMode::Repeat);
        float f1;
        if (zeus::close_enough(acos, 0.f)) {
          f1 = 0.f;
        } else {
          f1 = scale * std::sin(acos);
          scale *= std::cos(acos);
        }
        if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
          DrawDirect(item.GetColor(), f1, scale);
        } else {
          DrawStreamed(item.GetColor(), f1, scale);
        }
      }
    }
    ResetTevSwapMode(mgr);
  }
}

void CVisorFlare::SetupRenderState(const CStateManager& mgr) const {
  if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
    CGX::SetBlendMode(GX::BM_BLEND, GX::BL_ONE, GX::BL_ONE, GX::LO_CLEAR);
    CGX::SetStandardTevColorAlphaOp(GX::TEVSTAGE0);
    CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
    CGX::SetAlphaCompare(GX::ALWAYS, 0, GX::AOP_OR, GX::ALWAYS, 0);
    GXSetTevSwapMode(GX::TEVSTAGE0, GX::TEV_SWAP0, GX::TEV_SWAP1);
    CGX::SetTevKColorSel(GX::TEVSTAGE0, GX::TEV_KCSEL_K0_A);
    CGX::SetTevKAlphaSel(GX::TEVSTAGE0, GX::TEV_KASEL_K0_A);
    CGX::SetTevColorIn(GX::TEVSTAGE0, GX::CC_ZERO, GX::CC_TEXC, GX::CC_KONST, GX::CC_ZERO);
    CGX::SetTevAlphaIn(GX::TEVSTAGE0, GX::CA_ZERO, GX::CA_TEXA, GX::CA_KONST, GX::CA_ZERO);
    CGX::SetNumTexGens(1);
    CGX::SetNumChans(0);
    CGX::SetTexCoordGen(GX::TEXCOORD0, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
    if (x0_blendMode == EBlendMode::Blend) {
      CGX::SetStandardTevColorAlphaOp(GX::TEVSTAGE1);
      CGX::SetTevOrder(GX::TEVSTAGE1, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
      CGX::SetTevColorIn(GX::TEVSTAGE1, GX::CC_ZERO, GX::CC_TEXA, GX::CC_CPREV, GX::CC_ZERO);
      CGX::SetTevAlphaIn(GX::TEVSTAGE1, GX::CA_ZERO, GX::CA_TEXA, GX::CA_APREV, GX::CA_ZERO);
      CGX::SetNumTevStages(2);
    } else if (x0_blendMode == EBlendMode::Additive) {
      CGX::SetNumTevStages(1);
    }
    constexpr std::array vtxDescList{
        GX::VtxDescList{GX::VA_POS, GX::DIRECT},
        GX::VtxDescList{GX::VA_TEX0, GX::DIRECT},
        GX::VtxDescList{},
    };
    CGX::SetVtxDescv(vtxDescList.data());
  } else {
    if (x0_blendMode == EBlendMode::Blend) {
      g_Renderer->SetBlendMode_AlphaBlended();
    } else if (x0_blendMode == EBlendMode::Additive) {
      g_Renderer->SetBlendMode_AdditiveAlpha();
    }
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
  }
}

void CVisorFlare::ResetTevSwapMode(const CStateManager& mgr) const {
  if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
    GXSetTevSwapMode(GX::TEVSTAGE0, GX::TEV_SWAP0, GX::TEV_SWAP0);
  }
}

void CVisorFlare::DrawDirect(const zeus::CColor& color, float f1, float f2) const {
  zeus::CColor kcolor = color;
  kcolor.a() *= x24_;
  CGX::SetTevKColor(GX::KCOLOR0, kcolor);
  CGX::Begin(GX::TRIANGLESTRIP, GX::VTXFMT0, 4);
  GXPosition3f32(f1 - f2, 0.f, f2 + f1);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(f2 + f1, 0.f, f2 - f1);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(-(f2 - f1), 0.f, -(f2 - f1));
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(-f1 + f2, 0.f, -f2 - f1);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();
}

void CVisorFlare::DrawStreamed(const zeus::CColor& color, float f1, float f2) const {
  CGraphics::StreamBegin(GX::TRIANGLESTRIP);
  zeus::CColor kcolor = color;
  kcolor.a() *= x24_;
  CGraphics::StreamColor(kcolor);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(f1 - f2, 0.f, f2 + f1);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(f1 + f2, 0.f, f2 - f1);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(-(f1 + f2), 0.f, -(f2 - f1));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(-f1 + f2, 0.f, -f2 - f1);
  CGraphics::StreamEnd();
}

} // namespace metaforce
