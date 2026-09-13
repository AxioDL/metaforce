#include "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

const CVector3f CScriptCameraPitchVolume::skScaleFactor(0.5f, 0.5f, 0.5f);

CScriptCameraPitchVolume::CScriptCameraPitchVolume(TUniqueId uid, const bool active,
                                                   const rstl::string& name,
                                                   const CEntityInfo& info, const CVector3f& scale,
                                                   const CTransform4f& xf, const CRelAngle& upPitch,
                                                   const CRelAngle& downPitch,
                                                   float maxInterpDistance)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_obbox(xf, CVector3f::ByElementMultiply(scale, skScaleFactor))
, x124_upPitch(upPitch)
, x128_downPitch(downPitch)
, x12c_scale(CVector3f::ByElementMultiply(scale, skScaleFactor))
, x138_maxInterpDistance(maxInterpDistance)
, x13c_24_entered(false)
, x13c_25_occupied(false) {}

ENTITY_ACCEPT_IMPL(CScriptCameraPitchVolume)

rstl::optional_object< CAABox > CScriptCameraPitchVolume::GetTouchBounds() const {
  return xe8_obbox.CalculateAABox(CTransform4f::Identity());
}

void CScriptCameraPitchVolume::Touch(CActor& act, CStateManager& mgr) {
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(act)) {
    rstl::optional_object< CAABox > box = act.GetTouchBounds();
    if (box) {
      COBBox box2(COBBox::FromAABox(*box, CTransform4f::Identity()));
      x13c_24_entered = xe8_obbox.OBBIntersectsBox(box2);
    }
  }
}

void CScriptCameraPitchVolume::Think(float, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x13c_24_entered && !x13c_25_occupied) {
    Entered(mgr);
  }
  if (!x13c_24_entered && x13c_25_occupied) {
    Exited(mgr);
  }

  x13c_24_entered = false;
}

void CScriptCameraPitchVolume::Entered(CStateManager& mgr) {
  x13c_25_occupied = true;
  mgr.CameraManager()->FirstPersonCamera()->SetScriptPitchId(GetUniqueId());
}

void CScriptCameraPitchVolume::Exited(CStateManager& mgr) {
  x13c_25_occupied = false;
  mgr.CameraManager()->FirstPersonCamera()->SetScriptPitchId(kInvalidUniqueId);
}

CScriptCameraPitchVolume::~CScriptCameraPitchVolume() {}
