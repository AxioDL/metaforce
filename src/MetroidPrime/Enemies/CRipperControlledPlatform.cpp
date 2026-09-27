#include "MetroidPrime/Enemies/CRipperControlledPlatform.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameCollision.hpp"

#include "Kyoto/Math/CRelAngle.hpp"

CRipperControlledPlatform::CRipperControlledPlatform(
    TUniqueId uid, TUniqueId owner, const rstl::string& name, const CEntityInfo& info,
    const CTransform4f& xf, const CAABox& bounds, bool active,
    const rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > >& colTree)
: CScriptPlatform(uid, name, info, xf, CModelData::CModelDataNull(), CActorParameters::None(),
                  bounds, 0.f, false, 1.f, active, CHealthInfo(FLT_MAX, 10.f),
                  CDamageVulnerability::ImmuneVulnerability(), colTree, false, 1, 1)
, mOwner(owner)
, mYaw(GetYaw()) {}

float RCP_2PI = 0.15915494f;

CQuaternion CRipperControlledPlatform::Move(float arg, CStateManager& mgr) {
  if (const CActor* actor = static_cast< CActor* >(mgr.ObjectById(mOwner))) {
    CVector3f delta = actor->GetTranslation() - GetTranslation();
    MoveToWR(GetTranslation() + delta, arg);
    
    float zRot = CMath::ClampRadians(actor->GetYaw() - mYaw);
    if (zRot > M_PIF) {
      zRot -= M_2PIF;
    }
    const CQuaternion quat = CQuaternion::ZRotation(CRelAngle::FromRadians(zRot));
    RotateToOR(quat, arg);

    TEntityList nearList;
    CAABox volume(GetMotionVolume(arg));
    mgr.BuildColliderList(nearList, *this, volume);

    TEntityList filteredNearList;
    for (TEntityList::iterator id = nearList.begin(); id != nearList.end(); ++id) {
      if (!IsRider(*id) && !IsSlave(*id)) {
        filteredNearList.push_back(*id);
      }
    }

    SetMovable(true);
    CGameCollision::Move(mgr, *this, arg, &filteredNearList);
    SetMovable(false);
    mYaw = GetYaw();
    return quat;
  }
  return CQuaternion::NoRotation();
}

CRipperControlledPlatform::~CRipperControlledPlatform() {}
