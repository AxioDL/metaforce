#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CScriptActor::CScriptActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData, const CAABox& aabb,
                           const CMaterialList& matList, float mass, float zMomentum,
                           const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           const CActorParameters& actParms, bool looping, bool active,
                           uint shaderIdx, float xrayAlpha, const bool noThermalHotZ,
                           const bool castsShadow, const bool scaleAdvancementDelta,
                           const bool materialFlag54)
: CPhysicsActor(uid, active, name, info, xf, mData, matList, aabb, SMoverData(mass), actParms, 0.3f,
                0.1f)
, mInitialHealth(hInfo)
, mCurrentHealth(hInfo)
, mDamageVulnerability(dVuln)
, mFadeInTime(actParms.GetFadeInTime())
, mFadeOutTime(actParms.GetFadeOutTime())
, mShaderIdx(shaderIdx)
, mXrayAlpha(xrayAlpha)
, mTriggerId(kInvalidUniqueId)
, mNoThermalHotZ(noThermalHotZ)
, mDead(false)
, mAnimating(true)
, mXrayAlphaEnabled(!close_enough(xrayAlpha, 1.f))
, mInXrayAlpha(false)
, mProcessModelFlags(mXrayAlphaEnabled || mNoThermalHotZ ||
                            mShaderIdx != 0)
, mScaleAdvancementDelta(scaleAdvancementDelta)
, mMaterialFlag54(materialFlag54)
, mIsPlayerActor(false) {
  if (HasModelData()) {
    if (castsShadow) {
      SetDrawShadow(true);
    }

    if (HasAnimation()) {
      ModelData()->EnableLooping(looping);
    }
  }

  SetMomentumWR(CVector3f(0.f, 0.f, -zMomentum));
}

CScriptActor::~CScriptActor() {}

CHealthInfo* CScriptActor::HealthInfo(CStateManager&) { return &mCurrentHealth; }

const CDamageVulnerability* CScriptActor::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CScriptActor::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CScriptActor::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Solid)) {
    return CPhysicsActor::GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptActor::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (HasAnimation()) {
    const bool timeRemaining =
        GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, rstl::string_l("Whole Body"));
    const bool loop = GetModelData()->GetIsLoop();

    CAdvancementDeltas deltas = CActor::UpdateAnimation(dt, mgr, true);

    if (timeRemaining || loop) {
      mAnimating = true;

      if (mScaleAdvancementDelta) {
        CVector3f pos = GetTransform().TransposeRotate(deltas.GetOffsetDelta());
        CVector3f scale = GetModelData()->GetScale();
        pos = CVector3f(scale.GetX() * pos.GetX(), scale.GetY() * pos.GetY(),
                        scale.GetZ() * pos.GetZ());
        pos = GetTransform().Rotate(pos);
        MoveToOR(pos, dt);
      } else {
        MoveToOR(deltas.GetOffsetDelta(), dt);
      }

      RotateToOR(deltas.GetOrientationDelta(), dt);
    }

    if (!timeRemaining && mAnimating && !loop) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      mAnimating = false;
    }
  }

  if (!mDead && HealthInfo(mgr)->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  }
}

void CScriptActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea: {
    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->mState != kSS_InheritBounds || conn->mMsg != kSM_Activate) {
        continue;
      }

      CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(current->second))) {
          mTriggerId = current->second;
        }
        current++;
      }
    }

    if (mMaterialFlag54) {
      CActor::AddMaterial(kMT_Unknown54, mgr);
    }
    break;
  }
  case kSM_Reset: {
    mDead = false;
    mCurrentHealth = mInitialHealth;
    break;
  }
  case kSM_Increment: {
    if (!GetActive()) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Activate);
      CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), mFadeInTime);
    }
    break;
  }
  case kSM_Decrement: {
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), mFadeOutTime);
    break;
  }
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActor::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);

  if (GetPreRenderClipped() &&
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    SetPreRenderClipped(false);
  }

  if (!GetPreRenderClipped() && mProcessModelFlags) {
    if (mXrayAlphaEnabled) {
      CModelFlags xrayFlags = CModelFlags::AlphaBlended(mXrayAlpha);
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
        SetModelFlags(xrayFlags);
        mInXrayAlpha = true;
      } else if (mInXrayAlpha) {
        mInXrayAlpha = false;
        if (GetModelFlags() == xrayFlags) {
          SetModelFlags(CModelFlags::Normal());
        }
      }
    }

    if (mNoThermalHotZ && GetThermalFlags() == kTF_Hot) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
        SetModelFlags(GetModelFlags().DepthCompareUpdate(false, false));
      } else {
        SetModelFlags(GetModelFlags().DepthCompareUpdate(true, true));
      }
    }

    if (mShaderIdx != 0) {
      SetModelFlags(GetModelFlags().UseShaderSet(mShaderIdx));
    }
  }

  if (mgr.GetObjectById(mTriggerId) == nullptr) {
    mTriggerId = kInvalidUniqueId;
  }
}

ENTITY_ACCEPT_IMPL(CScriptActor)

EWeaponCollisionResponseTypes CScriptActor::GetCollisionResponseType(const CVector3f& v1,
                                                                     const CVector3f& v2,
                                                                     const CWeaponMode& wMode,
                                                                     int w) const {

  const CDamageVulnerability* dVuln = GetDamageVulnerability();
  if (dVuln->GetVulnerability(wMode, CDamageVulnerability::kRD_No) == kVN_Deflect) {
    const EDeflectionType deflectType = dVuln->GetDeflectionType(wMode);
    switch (deflectType) {
    case kDT_Ricochet:
    case kDT_RetargetPlayer:
    case kDT_RetargetPlayerCombo:
      return kWCR_Unknown15;
    default:
      break;
    }
  }
  return CActor::GetCollisionResponseType(v1, v2, wMode, w);
}

CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const {
  if (mTriggerId != kInvalidUniqueId) {
    const CScriptTrigger* trigger =
        static_cast< const CScriptTrigger* >(mgr.GetObjectById(mTriggerId));
    if (trigger) {
      return trigger->GetTriggerBoundsWR();
    }
  }

  return CActor::GetSortingBounds(mgr);
}
