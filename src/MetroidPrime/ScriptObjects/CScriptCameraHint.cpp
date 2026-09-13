#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include "rstl/algorithm.hpp"

CCameraOverrideInfo::CCameraOverrideInfo(
    uint overrideFlags, CBallCamera::EBallCameraBehaviour behaviour, float minDist, float maxDist,
    float backwardsDist, const CVector3f& lookAtOffset, const CVector3f& chaseLookAtOffset,
    const CVector3f& ballToCam, float fov, float attitudeRange, float azimuthRange,
    float anglePerSecond, float clampVelRange, float clampRotRange, float elevation,
    float interpolateTime, float clampVelTime, float controlInterpDur)
: mOverrideFlags(overrideFlags)
, mBehaviour(behaviour)
, mMinDist(minDist)
, mMaxDist(maxDist)
, mBackwardsDist(backwardsDist)
, mLookAtOffset(lookAtOffset)
, mChaseLookAtOffset(chaseLookAtOffset)
, mBallToCam(ballToCam)
, mFov(fov)
, mAttitudeRange(attitudeRange)
, mAzimuthRange(azimuthRange)
, mAnglePerSecond(anglePerSecond)
, mClampVelRange(clampVelRange)
, mClampRotRange(clampRotRange)
, mElevation(elevation)
, mInterpolateTime(interpolateTime)
, mClampVelTime(clampVelTime)
, mControlInterpDur(controlInterpDur) {}

CScriptCameraHint::CScriptCameraHint(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const bool active, int priority, CBallCamera::EBallCameraBehaviour behavior, int overrideFlags,
    float minDist, float maxDist, float backwardsDist, const CVector3f& lookAtOffset,
    const CVector3f& chaseLookAtOffset, const CVector3f& ballToCam, float fov, float attitudeRange,
    float azimuthRange, float anglePerSecond, float clampVelRange, float clampRotRange,
    float elevation, float interpolateTime, float clampVelTime, float controlInterpDur)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mPriority(priority)
, mOverrideInfo(overrideFlags, behavior, minDist, maxDist, backwardsDist, lookAtOffset,
                chaseLookAtOffset, ballToCam, fov, attitudeRange, azimuthRange, anglePerSecond,
                clampVelRange, clampRotRange, elevation, interpolateTime, clampVelTime,
                controlInterpDur)
, mSenders()
, mDelegatedCameraId(kInvalidUniqueId)
, mInactive(false)
, mOrigXf(xf) {}

CScriptCameraHint::~CScriptCameraHint() {}

TUniqueId CScriptCameraHint::GetSenderId() const {
  if (mSenders.size() > 0) {
    return mSenders.front();
  }
  return kInvalidUniqueId;
}

void CScriptCameraHint::ClearSenders() { mSenders.clear(); }

void CScriptCameraHint::AddSender(TUniqueId uid) {
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), uid);
  if (it == mSenders.end()) {
    mSenders.push_back(uid);
  }
}

void CScriptCameraHint::RemoveSender(TUniqueId uid, CStateManager& mgr) {
  if (mSenders.empty()) {
    return;
  }
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), uid);
  if (it == mSenders.end()) {
    mSenders.erase(mSenders.begin());
  } else {
    mSenders.erase(it);
  }
}

void CScriptCameraHint::CheckLegacyConnections(CStateManager& mgr) {
  TUniqueId uid = GetUniqueId();
  CObjectList& objList = mgr.ObjectListById(kOL_All);
  mDelegatedCameraId = kInvalidUniqueId;
  for (int idx = objList.GetFirstObjectIndex(); idx != -1; idx = objList.GetNextObjectIndex(idx)) {
    CEntity* entity = TCastToPtr< CEntity >(objList[idx]);
    if (!entity) {
      continue;
    }
    rstl::vector< SConnection >& connections = entity->ConnectionList();
    rstl::vector< SConnection >::iterator outer = connections.begin();
    for (; outer != connections.end(); ++outer) {
      if (mgr.GetIdForScript(outer->x8_objId) != uid ||
          (outer->x4_msg != kSM_Increment && outer->x4_msg != kSM_Decrement)) {
        continue;
      }
      rstl::vector< SConnection >::iterator inner = connections.begin();
      while (inner != connections.end()) {
        if (inner->x4_msg == kSM_Increment || inner->x4_msg == kSM_Decrement) {
          TUniqueId senderId = mgr.GetIdForScript(inner->x8_objId);
          CPathCamera* pathCamera = TCastToPtr< CPathCamera >(mgr.ObjectById(senderId));
          CScriptSpindleCamera* spindleCamera =
              TCastToPtr< CScriptSpindleCamera >(mgr.ObjectById(senderId));
          if (pathCamera || spindleCamera) {
            connections.erase(inner);
            if (mDelegatedCameraId != senderId) {
              mDelegatedCameraId = senderId;
            }
            break;
          } else {
            inner = inner + 1; // ?
          }
        } else {
          inner = inner + 1; // ?
        }
      }
      break;
    }
  }
}

void CScriptCameraHint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Deleted:
  case kSM_Deactivate: {
    CCameraManager* cameraManager = mgr.CameraManager();
    cameraManager->ReallyRemoveCameraHint(GetUniqueId(), mgr);
    break;
  }
  case kSM_InitializedInArea:
    CheckLegacyConnections(mgr);
    break;
  default:
    break;
  }

  if (GetActive()) {
    CCameraManager* cameraManager = mgr.CameraManager();
    switch (msg) {
    case kSM_Increment:
      AddSender(uid);
      cameraManager->AddCameraHint(GetUniqueId(), mgr);
      mInactive = false;
      break;
    case kSM_Decrement:
      RemoveSender(uid, mgr);
      cameraManager->DeleteCameraHint(GetUniqueId(), mgr);
      break;
    default:
      break;
    }
  }

  CCameraManager* cameraManager = mgr.CameraManager();
  switch (msg) {
  case kSM_Follow:
    if (!GetActive()) {
      SetActive(true);
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        CVector3f followerToThisFlat = mOrigXf.GetTranslation() - act->GetTranslation();
        followerToThisFlat.SetZ(0.f);
        if (followerToThisFlat.CanBeNormalized()) {
          followerToThisFlat.Normalize();
        } else {
          followerToThisFlat = act->GetTransform().GetColumn(kDY);
        }
        CVector3f pos = act->GetTranslation();
        pos.SetZ(mOrigXf.GetTranslation().GetZ());
        SetTransform(CTransform4f::LookAt(pos, pos + followerToThisFlat));
      }
    }
    AddSender(uid);
    cameraManager->AddCameraHint(GetUniqueId(), mgr);
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptCameraHint)
