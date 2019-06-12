#include "CScriptCameraPitchVolume.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "CPlayer.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "Particle/CGenDescription.hpp"
#include "TCastTo.hpp"

namespace urde {
const zeus::CVector3f CScriptCameraPitchVolume::skScaleFactor = zeus::CVector3f(0.5f);

CScriptCameraPitchVolume::CScriptCameraPitchVolume(TUniqueId uid, bool active, std::string_view name,
                                                   const CEntityInfo& info, const zeus::CVector3f& scale,
                                                   const zeus::CTransform& xf, const zeus::CRelAngle& r1,
                                                   const zeus::CRelAngle& r2, float maxInterpDistance)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_obbox(xf, scale * skScaleFactor)
, x124_upPitch(r1)
, x128_downPitch(r2)
, x12c_scale(scale * skScaleFactor)
, x138_maxInterpDistance(maxInterpDistance) {}

void CScriptCameraPitchVolume::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptCameraPitchVolume::Think(float, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x13c_24_entered && !x13c_25_occupied)
    Entered(mgr);
  else if (!x13c_24_entered && x13c_25_occupied)
    Exited(mgr);

  x13c_24_entered = false;
}

std::optional<zeus::CAABox> CScriptCameraPitchVolume::GetTouchBounds() const {
  return {xe8_obbox.calculateAABox(zeus::CTransform())};
}

void CScriptCameraPitchVolume::Touch(CActor& act, CStateManager& mgr) {
  TCastToPtr<CPlayer> pl(act);
  if (!pl)
    return;

  auto plBox = pl->GetTouchBounds();
  if (!plBox)
    return;

  x13c_24_entered = xe8_obbox.AABoxIntersectsBox(plBox.value());
}

void CScriptCameraPitchVolume::Entered(urde::CStateManager& mgr) {
  x13c_25_occupied = true;
  mgr.GetCameraManager()->GetFirstPersonCamera()->SetScriptPitchId(GetUniqueId());
}

void CScriptCameraPitchVolume::Exited(CStateManager& mgr) {
  x13c_25_occupied = false;
  mgr.GetCameraManager()->GetFirstPersonCamera()->SetScriptPitchId(kInvalidUniqueId);
}
} // namespace urde
