#include "Runtime/World/CVisorFlare.hpp"

#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CSimplePool.hpp"

namespace urde {

std::optional<CVisorFlare::CFlareDef> CVisorFlare::LoadFlareDef(CInputStream& in) {
  u32 propCount = in.readUint32Big();
  if (propCount != 4)
    return std::nullopt;

  CAssetId txtrId = in.readUint32Big();
  float f1 = in.readFloatBig();
  float f2 = in.readFloatBig();
  zeus::CColor color = zeus::CColor::ReadRGBABig(in);
  if (!txtrId.IsValid())
    return std::nullopt;

  TToken<CTexture> tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});

  return CFlareDef(tex, f1, f2, color);
}

CVisorFlare::CVisorFlare(EBlendMode blendMode, bool b1, float f1, float f2, float f3, u32 w1, u32 w2,
                         const std::vector<CFlareDef>& flares)
: x0_blendMode(blendMode)
, x4_flareDefs(flares)
, x14_b1(b1)
, x18_f1(std::max(f1, FLT_EPSILON))
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
    const float mag = (camDiff.magnitude());
    rstl::reserved_vector<TUniqueId, 1024> nearVec;
    mgr.BuildNearList(nearVec, camPos, camDiff * (1.f / mag), mag,
                      CMaterialFilter::MakeInclude({EMaterialTypes::Occluder}), act);
    TUniqueId id;
    CRayCastResult result = mgr.RayWorldIntersection(
        id, camPos, camDiff, mag,
        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::SeeThrough}), nearVec);

    if (result.IsValid()) {
      x28_ -= mag;
    } else {
      x28_ += mag;
    }
    x28_ = zeus::clamp(0.f, x28_, x18_f1);
    x24_ = 1.f - (x28_ / x18_f1);

    const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    x24_ *= std::max(0.f, 1.f - (4.f * x1c_f2 * (1.f - (pos - curCam->GetTranslation()).dot(curCam->GetTransform().basis[1]))));

    if (x2c_w1 == 2) {
      mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + x24_);
    }

    printf("%08X %f %f\n", act->GetEditorId().id, x24_, x28_);
  }
}

void CVisorFlare::Render(const zeus::CVector3f& pos, const CStateManager& mgr) const {
  if (zeus::close_enough(x28_, x18_f1) &&
      mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    int iVar;
    const auto visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
    if (visor == CPlayerState::EPlayerVisor::Thermal) {
      iVar = x2c_w1;
    } else if (visor == CPlayerState::EPlayerVisor::XRay) {
      return;
    } else {
      iVar = x30_w2;
    }
    if (iVar != 0)
      return;
    CGraphics::DisableAllLights();
    // g_Renderer->SetDepthReadWrite(false, false);
    //const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    //zeus::CVector3f camPos = cam->GetTranslation();
  }
}

} // namespace urde
