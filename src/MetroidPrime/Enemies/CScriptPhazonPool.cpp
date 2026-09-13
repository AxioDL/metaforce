#include "MetroidPrime/Enemies/CScriptPhazonPool.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include <Kyoto/Particles/CElementGen.hpp>

CScriptPhazonPool::CScriptPhazonPool(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CVector3f& scale, bool active, const CAssetId& w1, const CAssetId& w2, const CAssetId& w3,
    const CAssetId& w4, const uint p11, const CDamageInfo& dInfo, const CVector3f& orientedForce,
    const ETriggerFlags triggerFlags, const bool p15, const float p16, const float p17,
    const float p18, const float p19)
: CScriptTrigger(uid, name, info, xf.GetTranslation(), CAABox::Identity(), dInfo, orientedForce,
                 triggerFlags, active, false, false)
, mModelData1(rs_new CModelData(CStaticRes(w2, CVector3f(1.f, 1.f, 1.f))))
, mModelData2(rs_new CModelData(CStaticRes(w1, CVector3f(1.f, 1.f, 1.f))))
, mBounds(CAABox::Identity())
, mScale(scale)
, x19c(p16)
, x1a0(p16)
, x1a4(0.001f)
, mRotY(0.f)
, mRotZ(0.f)
, x1b0(0.f)
, x1b4(0.f)
, x1b8(0.f)
, x1bc(p17)
, x1c0(p19)
, x1c4(0.f)
, x1c8(p18)
, x1cc(0.f)
, x1d0(0.f)
, x1d4(0.f)
, x1d8(p11)
, x1dc(0)
, x1e0_24(p15)
, x1e0_25(false) {
  SetThermalFlags(kTF_Hot);
  if (w3 != kInvalidAssetId) {
    TLockedToken< CGenDescription > tok =
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', w3)));
    mElementGen1 = rs_new CElementGen(tok);
    if (!mElementGen1.null()) {
      mElementGen1->SetParticleEmission(false);
    }
  }

  if (w4 != kInvalidAssetId) {
    TLockedToken< CGenDescription > tok =
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', w4)));
    mElementGen2 = rs_new CElementGen(tok);
    if (!mElementGen2.null()) {
      mElementGen2->SetGlobalScale(CVector3f(mScale.GetX(), mScale.GetX(), mScale.GetX()));
      mElementGen2->SetParticleEmission(false);
    }
  }
}

CScriptPhazonPool::~CScriptPhazonPool() {}

void CScriptPhazonPool::Render(const CStateManager& mgr) const {
  CActor::Render(mgr);
  const CTransform4f& xf = GetTransform();

  bool depth = x1a4 > 0.25f;
  const CModelFlags flags = CModelFlags::AlphaBlendedDepthCompareUpdate(x1a4, depth, depth);

  if (!mModelData1.null()) {
    mModelData1->Render(mgr, xf * CTransform4f::RotateZ(mRotZ), nullptr, flags);
  }

  if (!mModelData2.null()) {
    const CQuaternion quat = CQuaternion::YXZRotation(mRotY, mRotZ, mRotZ);
    mModelData2->Render(mgr, xf * quat.BuildTransform4f(), nullptr, flags);
  }
}

void CScriptPhazonPool::UpdateParticleGens(CStateManager& mgr) {
  x1b0 = mgr.Random()->Range(-0.5f, 0.5f);
  x1b4 = mgr.Random()->Range(-1.f, 1.f);
  x1b8 = mgr.Random()->Range(0.25f, 2.f);
  x1cc = 0.f;
  x1d4 = 0.f;
  x1c4 = x1c0;
  x1a0 = x19c;
  x1dc = 1;
  x1e0_25 = false;

  if (!mElementGen1.null()) {
    mElementGen1->SetGlobalTranslation(GetTranslation());
    mElementGen1->SetParticleEmission(false);
  }
  if (!mElementGen2.null()) {
    mElementGen2->SetGlobalTranslation(GetTranslation());
    mElementGen2->SetParticleEmission(true);
  }
}

void CScriptPhazonPool::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);
  UpdateInhabitants(mgr);
  const CVector3f translation = GetTranslation();
  const CVector3f scale = (x1dc == 1 ? x1a4 : 1.f) * mScale;
  SetTransform(CTransform4f::FromRows(CVector3f(scale.GetX(), 0.f, 0.f),
                                      CVector3f(0.f, scale.GetY(), 0.f),
                                      CVector3f(0.f, 0.f, scale.GetZ()), translation));
  SetTriggerBounds(CAABox(CVector3f::ByElementMultiply(mBounds.GetMinPoint(), scale),
                          CVector3f::ByElementMultiply(mBounds.GetMaxPoint(), scale)));
  mRotY += dt * x1b0;
  mRotZ += dt * x1b4;
  if (mRotY > 6.2831855f) {
    mRotY = 0.f;
  }
  if (mRotZ > 6.2831855f) {
    mRotZ = 0.f;
  }

  if (!mElementGen1.null()) {
    mElementGen1->SetModulationColor(CColor(x1a4, x1a4, x1a4, x1a4));
    mElementGen1->SetGlobalScale(CVector3f(scale.GetX(), scale.GetX(), scale.GetX()));
    mElementGen1->Update(dt);
  }
  if (!mElementGen2.null()) {
    mElementGen2->Update(dt);
  }

  bool shouldFree = false;
  switch (x1dc) {
  case 1:
    x1d4 += dt;
    if (x1d4 > 2.f) {
      x1d4 = 2.f;
      x1a4 += dt * x1b8;
      if (x1a4 > 1.f) {
        x1a4 = 1.f;
        x1dc = 2;
        SetEmitParticles(true);
        if (!mElementGen2.null()) {
          mElementGen2->SetParticleEmission(false);
        }
      }
    }
    break;
  case 2: {
    float drainTime = 0.f;
    if (x1e0_24 || x1e0_25) {
      x1c4 -= dt;
      if (x1c4 <= 0.f) {
        x1c4 = 0.f;
        drainTime = dt;
      }
    }
    const float inhabitantDrain = x1bc * (dt * x1cc);
    x1a0 -= inhabitantDrain + drainTime;
    x1a4 = x1a0 / x19c;
    if (x1a4 < 0.001f) {
      if (x1e0_24) {
        shouldFree = true;
      } else {
        SetCallTouch(false);
        if (!x1e0_25) {
          x1dc = 3;
          x1d0 = x1c8;
        } else {
          x1dc = 0;
          CActor::SetActive(false);
        }
      }
      SetEmitParticles(false);
    }
    x1cc = 0.f;
    break;
  }
  case 3:
    x1d0 -= dt;
    if (x1d0 <= 0.f) {
      x1d0 = 0.f;
      UpdateParticleGens(mgr);
    }
    break;
  }
  if (shouldFree) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CScriptPhazonPool::Touch(CActor& actor, CStateManager& mgr) {
  if (!GetActive() || x1dc != 2) {
    return;
  }

  CScriptTrigger::Touch(actor, mgr);
  if (actor.GetMaterialList().HasMaterial(kMT_Trigger)) {
    return;
  }

  for (AUTO(it, mInhabitants.begin()); it != mInhabitants.end(); ++it) {
    if (it->first == actor.GetUniqueId()) {
      it->second = true;
      return;
    }
  }

  const rstl::optional_object< CAABox > bounds = actor.GetTouchBounds();
  if (!bounds) {
    return;
  }
  mInhabitants.push_back(rstl::pair< TUniqueId, bool >(actor.GetUniqueId(), true));
  mgr.DeliverScriptMsg(&actor, GetUniqueId(), kSM_AddPhazonPoolInhabitant);
}

rstl::optional_object< CAABox > CScriptPhazonPool::GetTouchBounds() const {
  return CScriptTrigger::GetTouchBounds();
}

void CScriptPhazonPool::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (!mElementGen1.null()) {
      mElementGen1->SetGlobalTranslation(GetTranslation());
    }
    mBounds =
        !mModelData1.null() ? mModelData1->GetBounds() : CAABox(0.5f * -mScale, 0.5f * mScale);
    SetTriggerBounds(CAABox(CVector3f::ByElementMultiply(mBounds.GetMinPoint(), mScale),
                            CVector3f::ByElementMultiply(mBounds.GetMaxPoint(), mScale)));
    break;
  case kSM_Decrement:
    if (x1dc == 2) {
      x1cc += 1.f;
    }
    break;
  case kSM_Activate:
  case kSM_Open:
    UpdateParticleGens(mgr);
    break;
  case kSM_Close:
    if (x1e0_25 != true) {
      x1e0_25 = true;
      x1c4 = 0.f;
      SetEmitParticles(false);
    }
    break;
  case kSM_Deleted:
    RemoveInhabitants(mgr);
    break;
  }
  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptPhazonPool)

void CScriptPhazonPool::AddToRenderer(const CFrustumPlanes& frustum,
                                      const CStateManager& mgr) const {
  if (GetActive()) {
    if (!mElementGen1.null()) {
      gpRender->AddParticleGen(*mElementGen1);
    }
    if (!mElementGen2.null()) {
      gpRender->AddParticleGen(*mElementGen2);
    }
  }
  CActor::AddToRenderer(frustum, mgr);
  CActor::EnsureRendered(mgr);
}

void CScriptPhazonPool::UpdateInhabitants(CStateManager& mgr) {
  AUTO(it, mInhabitants.begin());
  while (it != mInhabitants.end()) {
    AUTO(next, it);
    ++next;
    CActor* const actor = TCastToPtr< CActor >(mgr.ObjectById(it->first));
    if (actor) {
      const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
      if (bounds) {
        GetTriggerBoundsWR();
      }
    }

    if (actor && it->second) {
      mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_UpdatePhazonPoolInhabitant);
      it->second = false;
    } else {
      mInhabitants.erase(it);
      if (actor) {
        mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_RemovePhazonPoolInhabitant);
      }
    }
    it = next;
  }
}

void CScriptPhazonPool::RemoveInhabitants(CStateManager& mgr) {
  AUTO(it, mInhabitants.begin());
  while (it != mInhabitants.end()) {
    AUTO(next, it);
    ++next;
    if (CActor* const actor = TCastToPtr< CActor >(mgr.ObjectById(it->first))) {
      mInhabitants.erase(it);
      mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_RemovePhazonPoolInhabitant);
    }
    it = next;
  }
}

void CScriptPhazonPool::SetEmitParticles(const bool val) {
  if (!mElementGen1.null()) {
    mElementGen1->SetParticleEmission(val);
  }
}
