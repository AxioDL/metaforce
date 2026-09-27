#include "MetroidPrime/CCollisionActorManager.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include <float.h>

CJointCollisionDescription CJointCollisionDescription::SphereCollision(CSegId pivotId, float radius,
                                                                       const rstl::string& name,
                                                                       float mass) {

  return CJointCollisionDescription(kCT_Sphere, pivotId, CSegId::Invalid(), CVector3f::Zero(),
                                    CVector3f::Zero(), radius, 0.f, kOT_Zero, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::SphereSubdivideCollision(
    CSegId pivotId, CSegId nextId, float radius, float maxSeparation, EOrientationType orientType,
    const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_SphereSubdivide, pivotId, nextId, CVector3f::Zero(),
                                    CVector3f::Zero(), radius, maxSeparation, orientType, name,
                                    mass);
}

CJointCollisionDescription CJointCollisionDescription::AABoxCollision(CSegId pivotId,
                                                                      const CVector3f& bounds,
                                                                      const rstl::string& name,
                                                                      float mass) {
  return CJointCollisionDescription(kCT_AABox, pivotId, CSegId::Invalid(), bounds,
                                    CVector3f::Zero(), 0.f, 0.f, kOT_Zero, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBAutoSizeCollision(
    CSegId pivotId, CSegId nextId, const CVector3f& bounds, EOrientationType orientType,
    const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_OBBAutoSize, pivotId, nextId, bounds, CVector3f::Zero(),
                                    0.f, 0.f, orientType, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBCollision(CSegId pivotId,
                                                                    const CVector3f& bounds,
                                                                    const CVector3f& pivotPoint,
                                                                    const rstl::string& name,
                                                                    float mass) {

  return CJointCollisionDescription(kCT_OBB, pivotId, CSegId::Invalid(), bounds, pivotPoint, 0.f,
                                    0.f, kOT_Zero, name, mass);
}

void CJointCollisionDescription::ScaleAllBounds(const CVector3f& scale) {
  mBounds = CVector3f::ByElementMultiply(scale, mBounds);
  mRadius *= scale.GetX();
  mMaxSeparation *= scale.GetX();
  mPivotPoint = CVector3f::ByElementMultiply(scale, mPivotPoint);
}

CCollisionActorManager::CCollisionActorManager(
    CStateManager& mgr, TUniqueId owner, TAreaId areaId,
    const rstl::vector< CJointCollisionDescription >& descs, bool active)
: mOwnerId(owner)
, mActive(active)
, mDestroyed(false)
, mMovable(true) {
  const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwnerId));
  if (act != nullptr) {
    const CAnimData* const animData = act->GetAnimationData();
    const CTransform4f worldXf = act->GetTransform();
    const CVector3f scale = act->GetModelScale();
    const CTransform4f scaleXf = CTransform4f::Scale(scale);
    mJointDescriptions.reserve(descs.size());
    for (AUTO(it, descs.begin()); it != descs.end(); ++it) {
      CJointCollisionDescription desc = *it;
      desc.ScaleAllBounds(scale);
      const CTransform4f pivotXf =
          GetWRLocatorTransform(*animData, desc.GetPivotId(), worldXf, scaleXf);
      if (desc.GetNextId() != CSegId::Invalid()) {
        const CTransform4f nextXf =
            GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
        const float distance = (nextXf.GetTranslation() - pivotXf.GetTranslation()).Magnitude();
        if (desc.GetType() == CJointCollisionDescription::kCT_OBBAutoSize) {
          if (distance <= FLT_EPSILON) {
            continue;
          }
          CVector3f bounds(desc.GetBounds()[kDX], distance + desc.GetBounds()[kDY],
                           desc.GetBounds()[kDZ]);
          TUniqueId uid = mgr.AllocateUniqueId();
          CCollisionActor* const colAct =
              rs_new CCollisionActor(uid, areaId, mOwnerId, bounds,
                                     CVector3f(0.f, 0.5f * distance, 0.f), active, desc.GetMass());
          if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Zero) {
            colAct->SetTransform(pivotXf);
          } else {
            CVector3f up = pivotXf.GetUp();
            const CVector3f delta =
                (nextXf.GetTranslation() - pivotXf.GetTranslation()).AsNormalized();
            if (fabs(1.f - CMath::AbsF(CVector3f::Dot(delta, up))) < 100.0 * FLT_EPSILON) {
              up = pivotXf.GetForward();
            }
            const CTransform4f xf = CTransform4f::LookAt(pivotXf.GetTranslation(),
                                                         pivotXf.GetTranslation() + delta, up);
            colAct->SetTransform(xf);
          }
          mgr.AddObject(*colAct);
          mJointDescriptions.push_back(*it);
          (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
        } else {
          TUniqueId uid = mgr.AllocateUniqueId();
          CCollisionActor* const colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, active,
                                                                 desc.GetRadius(), desc.GetMass());
          colAct->SetTransform(pivotXf);
          mgr.AddObject(*colAct);
          mJointDescriptions.push_back(CJointCollisionDescription::SphereCollision(
              desc.GetPivotId(), desc.GetRadius(), desc.GetName(), 0.001f));
          (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
          const uint numSeps = CCast::ToUint32(distance / desc.GetMaxSeparation());
          if (numSeps != 0) {
            mJointDescriptions.reserve(mJointDescriptions.capacity() + numSeps);
            const float pitch = distance / float(numSeps + 1);
            for (uint i = 0; i < numSeps; ++i) {
              const float separation = pitch * float(i + 1);
              mJointDescriptions.push_back(CJointCollisionDescription::SphereSubdivideCollision(
                  desc.GetPivotId(), desc.GetNextId(), desc.GetRadius(), separation,
                  CJointCollisionDescription::kOT_One, desc.GetName(), 0.001f));
              TUniqueId newId = mgr.AllocateUniqueId();
              CCollisionActor* const newAct = rs_new CCollisionActor(
                  newId, areaId, mOwnerId, active, desc.GetRadius(), desc.GetMass());
              if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Zero) {
                const CTransform4f xf = CTransform4f::Translate(pivotXf.GetTranslation() +
                                                                separation * pivotXf.GetForward());
                newAct->SetTransform(xf);
              } else {
                CVector3f up = pivotXf.GetUp();
                const CVector3f delta =
                    (nextXf.GetTranslation() - pivotXf.GetTranslation()).AsNormalized();
                if (fabs(1.f - CMath::AbsF(CVector3f::Dot(delta, up))) < 100.0 * FLT_EPSILON) {
                  up = pivotXf.GetForward();
                }
                const CTransform4f xf = CTransform4f::Translate(
                    pivotXf.GetTranslation() +
                    separation * CTransform4f::LookAt(CVector3f::Zero(), delta, up).GetForward());
                newAct->SetTransform(xf);
              }
              mgr.AddObject(*newAct);
              (mJointDescriptions.end() - 1)->SetCollisionActorId(newId);
            }
          }
        }
      } else {
        TUniqueId uid = mgr.AllocateUniqueId();
        CCollisionActor* colAct = nullptr;
        if (desc.GetType() == CJointCollisionDescription::kCT_Sphere) {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, active, desc.GetRadius(),
                                          desc.GetMass());
        } else if (desc.GetType() == CJointCollisionDescription::kCT_OBB) {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, desc.GetBounds(),
                                          desc.GetPivotPoint(), active, desc.GetMass());
        } else {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, desc.GetBounds(), active,
                                          desc.GetMass());
        }
        colAct->SetTransform(pivotXf);
        mgr.AddObject(*colAct);
        mJointDescriptions.push_back(*it);
        (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
      }
    }
  }
}

CCollisionActorManager::~CCollisionActorManager() {}

void CCollisionActorManager::Update(float dt, CStateManager& mgr, EUpdateOptions updateOptions) {
  if (!mMovable) {
    SetMovable(mgr, true);
  }
  if (mActive) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwnerId));
    if (act != nullptr) {
      const CAnimData* animData = act->GetAnimationData();
      const CTransform4f worldXf = act->GetTransform();
      const CTransform4f scaleXf = CTransform4f::Scale(act->GetModelScale());
      for (int i = 0; i < mJointDescriptions.size(); ++i) {
        const CJointCollisionDescription& desc = mJointDescriptions[i];
        CCollisionActor* const colAct =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()));
        if (colAct != nullptr) {
          const CTransform4f pivotXf =
              GetWRLocatorTransform(*animData, desc.GetPivotId(), worldXf, scaleXf);
          CVector3f origin = pivotXf.GetTranslation();
          if (desc.GetType() == CJointCollisionDescription::kCT_OBB ||
              desc.GetType() == CJointCollisionDescription::kCT_OBBAutoSize) {
            if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Zero) {
              colAct->SetRotation(CQuaternion::FromMatrix(pivotXf));
            } else {
              const CTransform4f nextXf =
                  GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
              colAct->SetRotation(CQuaternion::FromMatrix(CTransform4f::LookAt(
                  pivotXf.GetTranslation(), nextXf.GetTranslation(), pivotXf.GetUp())));
            }
          } else if (desc.GetType() == CJointCollisionDescription::kCT_SphereSubdivide) {
            if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Zero) {
              origin += desc.GetMaxSeparation() * pivotXf.GetForward();
            } else {
              const CTransform4f nextXf =
                  GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
              origin += CTransform4f::LookAt(origin, nextXf.GetTranslation(), pivotXf.GetUp())
                            .GetForward() *
                        desc.GetMaxSeparation();
            }
          }
          if (updateOptions == kUO_ObjectSpace) {
            const CVector3f movement = colAct->GetTransform().TransposeMultiply(origin);
            colAct->MoveToOR(movement, dt);
          } else {
            colAct->SetTranslation(origin);
          }
        }
      }
    }
  }
}

void CCollisionActorManager::Destroy(CStateManager& mgr) const {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    mgr.DeleteObjectRequest(mJointDescriptions[i].GetCollisionActorId());
  }

  mDestroyed = true;
}

uchar CCollisionActorManager::GetActive() const { return mActive; }

void CCollisionActorManager::SetActive(CStateManager& mgr, bool active) {
  mActive = active;
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CActor* act =
        static_cast< CActor* >(mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()));
    if (act != nullptr) {
      if (active != act->GetActive()) {
        act->SetActive(active);
        if (active != false) {
          Update(0.f, mgr, kUO_WorldSpace);
        }
      }
    }
  }
}

void CCollisionActorManager::AddMaterial(CStateManager& mgr, const CMaterialList& list) {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CActor* act =
        TCastToPtr< CActor >(mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()));
    if (act != nullptr) {
      act->AddMaterial(list);
    }
  }
}

uint CCollisionActorManager::GetNumCollisionActors() const { return mJointDescriptions.size(); }

const CJointCollisionDescription& CCollisionActorManager::GetCollisionDescFromIndex(uint i) const {
  return mJointDescriptions[i];
}

rstl::optional_object< CVector3f > CCollisionActorManager::GetDeviation(const CStateManager& mgr,
                                                                        CSegId seg) const {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    const CJointCollisionDescription& desc = mJointDescriptions[i];
    if (desc.GetPivotId() != seg) {
      continue;
    }
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwnerId));
    if (act == nullptr) {
      continue;
    }
    const CCollisionActor* const colAct =
        TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(desc.GetCollisionActorId()));
    if (colAct == nullptr) {
      continue;
    }
    const CAnimData* const animData = act->GetAnimationData();
    const CTransform4f worldXf = act->GetTransform();
    const CTransform4f scaleXf = CTransform4f::Scale(act->GetModelScale());
    const CTransform4f locatorXf =
        GetWRLocatorTransform(*animData, desc.GetPivotId(), worldXf, scaleXf);
    return colAct->GetTranslation() - locatorXf.GetTranslation();
  }
  return rstl::optional_object_null();
}

CTransform4f CCollisionActorManager::GetWRLocatorTransform(const CAnimData& animData, CSegId id,
                                                           const CTransform4f& worldXf,
                                                           const CTransform4f& localXf) {
  CTransform4f locatorXf = animData.GetLocatorTransform(id, nullptr);
  const CVector3f origin = worldXf * (localXf * locatorXf.GetTranslation());
  locatorXf = worldXf.MultiplyIgnoreTranslation(locatorXf);
  locatorXf.SetTranslation(origin);
  return locatorXf;
}

void CCollisionActorManager::SetMovable(CStateManager& mgr, bool movable) {
  if (movable != mMovable) {
    mMovable = movable;
    for (int i = 0; i < mJointDescriptions.size(); ++i) {
      if (CCollisionActor* act = TCastToPtr< CCollisionActor >(
              mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()))) {
        act->SetMovable(mMovable);
        act->SetUseInSortedLists(mMovable);
      }
    }
  }
}
