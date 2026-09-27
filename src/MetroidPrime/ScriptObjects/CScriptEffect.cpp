#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"
uint CScriptEffect::mNumParticlesDrawing = 0;
uint CScriptEffect::mNumParticlesUpdating = 0;

static inline CTransform4f ClearTrans(const CTransform4f& xf) {
  CTransform4f ret = xf;
  ret.SetTranslation(CVector3f::Zero());
  return ret;
}

CScriptEffect::CScriptEffect(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CVector3f& scale, const CAssetId partId,
                             const CAssetId elscId, const bool hotInThermal,
                             const bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate,
                             const bool active, const bool useRateInverseCamDist,
                             float rateInverseCamDist, const float rateInverseCamDistRate,
                             const float duration, const float durationResetWhileVisible,
                             const bool useRateCamDistRange, const float rateCamDistRangeMin,
                             const float rateCamDistRangeMax, const float rateCamDistRangeFarRate,
                             const bool combatVisorVisible, const bool thermalVisorVisible,
                             const bool xrayVisorVisible, const CLightParameters& lParms,
                             const bool dieWhenSystemsDone)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, mElectricToken(nullptr)
, mParticleSystemToken(nullptr)
, mLightId(kInvalidUniqueId)
, mPartId(partId)
, mEnable(active)
, mNoTimerUnlessAreaOccluded(noTimerUnlessAreaOccluded)
, mRebuildSystemsOnActivate(rebuildSystemsOnActivate)
, mUseRateInverseCamDist(useRateInverseCamDist)
, mCombatVisorVisible(combatVisorVisible)
, mThermalVisorVisible(thermalVisorVisible)
, mXrayVisorVisible(xrayVisorVisible)
, mAnyVisorVisible(xrayVisorVisible && thermalVisorVisible && combatVisorVisible)
, mUseRateCamDistRange(useRateCamDistRange)
, mDieWhenSystemsDone(dieWhenSystemsDone)
, mCanRender(false)
, mRateInverseCamDist(rateInverseCamDist)
, mRateInverseCamDistSq(rateInverseCamDist * rateInverseCamDist)
, mRateInverseCamDistRate(rateInverseCamDistRate)
, mRateCamDistRangeMin(rateCamDistRangeMin)
, mRateCamDistRangeMax(rateCamDistRangeMax)
, mRateCamDistRangeFarRate(rateCamDistRangeFarRate)
, mRemTime(duration)
, mDuration(duration)
, mDurationResetWhileVisible(durationResetWhileVisible)
, mActorLights(lParms.MakeActorLights().release())
, mTriggerId(kInvalidUniqueId)
, mDestroyDelayTimer(0.f) {
  if (partId != kInvalidAssetId) {
    mParticleSystemToken = gpSimplePool->GetObj(SObjectTag('PART', partId));
    mParticleSystem = rs_new CElementGen(mParticleSystemToken);
    mParticleSystem->SetOrientation(ClearTrans(xf));
    mParticleSystem->SetGlobalTranslation(xf.GetTranslation());
    mParticleSystem->SetGlobalScale(scale);
    mParticleSystem->SetParticleEmission(active);
    mParticleSystem->SetModulationColor(lParms.GetAmbientColor());
    mParticleSystem->SetLeaveLightsEnabledForModelRender(mActorLights.get() != nullptr);
  }
  if (elscId != kInvalidAssetId) {
    mElectricToken = gpSimplePool->GetObj(SObjectTag('ELSC', elscId));
    mElectric = rs_new CParticleElectric(mElectricToken);
    mElectric->SetOrientation(ClearTrans(xf));
    mElectric->SetGlobalTranslation(xf.GetTranslation());
    mElectric->SetGlobalScale(scale);
    mElectric->SetParticleEmission(active);
    mElectric->SetModulationColor(lParms.GetAmbientColor());
  }
  SetDrawEnabled(true);
}

void CScriptEffect::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    if (mParticleSystem.get()) {
      mParticleSystem->SetOrientation(ClearTrans(GetTransform()));
      mParticleSystem->SetGlobalTranslation(GetTranslation());
    }
    if (mElectric.get()) {
      mElectric->SetOrientation(ClearTrans(GetTransform()));
      mElectric->SetGlobalTranslation(GetTranslation());
    }
    if (CActor* light = TCastToPtr< CActor >(mgr.ObjectById(mLightId))) {
      light->SetTransform(GetTransform());
    }
    SetTransformDirtySpare(false);
  }
  if (!mNoTimerUnlessAreaOccluded) {
    if (mRemTime <= 0.f) {
      return;
    }
  } else {
    if (mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
            CGameArea::kOS_Occluded &&
        mRemTime <= 0.f) {
      return;
    }
  }
  mRemTime -= dt;
  if (mEnable) {
    if (mParticleSystem.get()) {
      mParticleSystem->Update(dt);
      mNumParticlesUpdating += mParticleSystem->GetParticleCountAll();
    }
    if (mElectric.get()) {
      mElectric->Update(dt);
      mNumParticlesUpdating += mElectric->GetParticleCount();
    }
    if (mLightId != kInvalidUniqueId) {
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
        if (GetActive()) {
          light->SetLight(mParticleSystem->GetLight());
        }
      }
    }
    if (mDieWhenSystemsDone) {
      mDestroyDelayTimer += dt;
      if (mDestroyDelayTimer > 15.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
      if (AreBothSystemsDeleteable()) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
  }
  if (mParticleSystem.get()) {
    if (GetModelFlags().GetTrans() != 0) {
      mParticleSystem->SetModulationColor(GetModelFlags().GetColorRef());
    } else {
      mParticleSystem->SetModulationColor(CColor(0xffffffff));
    }
  }
}

void CScriptEffect::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool oldActive = GetActive();
  switch (msg) {
  case kSM_InitializedInArea:
    for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
      if ((conn->mState == kSS_Modify && conn->mMsg == kSM_Follow) ||
          (conn->mState == kSS_InheritBounds && conn->mMsg == kSM_Activate)) {
        CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->mObjId);
        for (AUTO(it, ids.first); it != ids.second; ++it) {
          if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(it->second))) {
            mTriggerId = it->second;
          }
        }
      }
    }
    break;
  case kSM_Registered:
    if (mParticleSystem.get() && mParticleSystem->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const CAssetId sourceId = mPartId;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("EffectPLight_") + GetDebugName(),
                                      GetTransform(), GetUniqueId(),
                                      mParticleSystem->GetLight(), sourceId, 1, 0.f));
    }
    break;
  case kSM_Deleted:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  case kSM_Activate:
    if (mRebuildSystemsOnActivate) {
      if (mParticleSystem.get()) {
        const CVector3f scale = mParticleSystem->GetGlobalScale();
        const CColor color = mParticleSystem->GetModulationColor();
        mParticleSystem = rs_new CElementGen(mParticleSystemToken);
        mParticleSystem->SetOrientation(ClearTrans(GetTransform()));
        mParticleSystem->SetGlobalTranslation(GetTranslation());
        mParticleSystem->SetGlobalScale(scale);
        mParticleSystem->SetLeaveLightsEnabledForModelRender(mActorLights.get() != nullptr);
        mParticleSystem->SetModulationColor(color);
      }
      if (mElectric.get()) {
        const CVector3f scale = mElectric->GetGlobalScale();
        const CColor color = mElectric->GetModulationColor();
        mElectric = rs_new CParticleElectric(mElectricToken);
        mElectric->SetOrientation(ClearTrans(GetTransform()));
        mElectric->SetGlobalTranslation(GetTranslation());
        mElectric->SetGlobalScale(scale);
        mElectric->SetModulationColor(color);
      }
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
  CActor* light = TCastToPtr< CActor >(mgr.ObjectById(mLightId));
  mgr.DeliverScriptMsg(light, uid, msg);
  if (oldActive != GetActive()) {
    if (GetActive()) {
      rstl::vector< TUniqueId > playIds;
      playIds.reserve(GetConnectionList().size());
      for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
        if (conn->mState == kSS_Play && conn->mMsg == kSM_Activate) {
          TUniqueId id = mgr.GetIdForScript(conn->mObjId);
          if (id != kInvalidUniqueId) {
            playIds.push_back(id);
          }
        }
      }
      if (playIds.size() != 0) {
        if (const CActor* const other = TCastToConstPtr< CActor >(mgr.GetObjectById(
                playIds[static_cast< int >(0.99f * (mgr.Random()->Float() * playIds.size()))]))) {
          SetTransform(other->GetTransform());
          if (light) {
            light->SetTransform(other->GetTransform());
          }
        }
      }
    }
    mEnable = true;
    if (mParticleSystem.get()) {
      mParticleSystem->SetParticleEmission(GetActive());
    }
    if (mElectric.get()) {
      mElectric->SetParticleEmission(GetActive());
    }
    if (GetActive()) {
      mRemTime = rstl::max_val(mDuration, mRemTime);
    }
  }
}

void CScriptEffect::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!mCanRender) {
    mRemTime = rstl::max_val(mDurationResetWhileVisible, mRemTime);
    return;
  }
  const CAABox& bounds = GetRenderBoundsCached();
  if (!frustum.BoxInFrustumPlanes(bounds)) {
    return;
  }
  mRemTime = rstl::max_val(mDurationResetWhileVisible, mRemTime);
  bool visible = true;
  if (!mAnyVisorVisible) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      visible = mCombatVisorVisible;
      break;
    case CPlayerState::kPV_XRay:
      visible = mXrayVisorVisible;
      break;
    case CPlayerState::kPV_Thermal:
      visible = mThermalVisorVisible;
      break;
    }
  }
  if (visible) {
    if (mActorLights.get()) {
      const CVector3f center = bounds.GetCenterPoint();
      mActorLights->BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
                                           CAABox(center, center));
      mActorLights->BuildDynamicLightList(mgr, bounds);
    }
    EnsureRendered(mgr);
  }
}

void CScriptEffect::Render(const CStateManager& mgr) const {
  if (mActorLights.get()) {
    mActorLights->ActivateLights();
  }
  if (mParticleSystem.get()) {
    const int count = mParticleSystem->GetParticleCountAll();
    if (count > 0) {
      mNumParticlesDrawing += count;
      mParticleSystem->Render();
    }
  }
  if (mElectric.get()) {
    const int count = mElectric->GetParticleCount();
    if (count > 0) {
      mNumParticlesDrawing += count;
      mElectric->Render();
    }
  }
}

void CScriptEffect::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mUseRateInverseCamDist || mUseRateCamDistRange) {
    float genRate = 1.f;
    const float camMagSq =
        (mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation() - GetTranslation())
            .MagSquared();
    const float camMag = camMagSq > 0.001f ? CMath::FastSqrtF(camMagSq) : 0.f;
    if (mUseRateInverseCamDist && camMagSq < mRateInverseCamDistSq) {
      genRate = (1.f - mRateInverseCamDistRate) * (camMag / mRateInverseCamDist) +
                mRateInverseCamDistRate;
    }
    if (mUseRateCamDistRange) {
      const float range = mRateCamDistRangeMax - mRateCamDistRangeMin;
      const float t =
          rstl::min_val(1.f, rstl::max_val(0.f, camMag - mRateCamDistRangeMin) / range);
      genRate = (1.f - t) * genRate + t * mRateCamDistRangeFarRate;
    }
    mParticleSystem->SetGeneratorRate(genRate);
  }
  if (!mgr.GetObjectById(mTriggerId)) {
    mTriggerId = kInvalidUniqueId;
  }
}

ENTITY_ACCEPT_IMPL(CScriptEffect)

void CScriptEffect::ResetParticleCounts() {
  mNumParticlesDrawing = 0;
  mNumParticlesUpdating = 0;
}

bool CScriptEffect::AreBothSystemsDeleteable() const {
  return (!mParticleSystem.get() || mParticleSystem->IsSystemDeletable()) &&
         (!mElectric.get() || mElectric->IsSystemDeletable());
}

bool CScriptEffect::CanRenderUnsorted(const CStateManager& mgr) const { return false; }

void CScriptEffect::CalculateRenderBounds() {
  const rstl::optional_object< CAABox > particleBounds = mParticleSystem.get()
                                                             ? mParticleSystem->GetBounds()
                                                             : rstl::optional_object< CAABox >();
  const rstl::optional_object< CAABox > electricBounds =
      mElectric.get() ? mElectric->GetBounds() : rstl::optional_object< CAABox >();
  if (particleBounds.valid() || electricBounds.valid()) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    if (particleBounds.valid()) {
      bounds.Include(*particleBounds);
    }
    if (electricBounds.valid()) {
      bounds.Include(*electricBounds);
    }
    SetRenderBounds(bounds);
    mCanRender = true;
  } else {
    const CVector3f translation = GetTranslation();
    SetRenderBounds(CAABox(translation, translation));
    mCanRender = false;
  }
}

CAABox CScriptEffect::GetSortingBounds(const CStateManager& mgr) const {
  if (mTriggerId != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(mTriggerId))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return GetRenderBoundsCached();
}

void CScriptEffect::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

CScriptEffect::~CScriptEffect() {}
