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
, mObbox(xf, CVector3f::ByElementMultiply(scale, skScaleFactor))
, mUpPitch(upPitch)
, mDownPitch(downPitch)
, mScale(CVector3f::ByElementMultiply(scale, skScaleFactor))
, mMaxInterpDistance(maxInterpDistance)
, mEntered(false)
, mOccupied(false) {}

ENTITY_ACCEPT_IMPL(CScriptCameraPitchVolume)

rstl::optional_object< CAABox > CScriptCameraPitchVolume::GetTouchBounds() const {
  return mObbox.CalculateAABox(CTransform4f::Identity());
}

void CScriptCameraPitchVolume::Touch(CActor& act, CStateManager& mgr) {
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(act)) {
    rstl::optional_object< CAABox > box = act.GetTouchBounds();
    if (box) {
      COBBox box2(COBBox::FromAABox(*box, CTransform4f::Identity()));
      mEntered = mObbox.OBBIntersectsBox(box2);
    }
  }
}

void CScriptCameraPitchVolume::Think(float, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mEntered && !mOccupied) {
    Entered(mgr);
  }
  if (!mEntered && mOccupied) {
    Exited(mgr);
  }

  mEntered = false;
}

void CScriptCameraPitchVolume::Entered(CStateManager& mgr) {
  mOccupied = true;
  mgr.CameraManager()->FirstPersonCamera()->SetScriptPitchId(GetUniqueId());
}

void CScriptCameraPitchVolume::Exited(CStateManager& mgr) {
  mOccupied = false;
  mgr.CameraManager()->FirstPersonCamera()->SetScriptPitchId(kInvalidUniqueId);
}
