#include "MetroidPrime/CGameLight.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CGameLight::CGameLight(TUniqueId uid, TAreaId aid, const bool active, const rstl::string& name,
                       const CTransform4f& xf, TUniqueId parentId, const CLight& light,
                       uint sourceId, uint priority, float lifeTime)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic), CActorParameters::None(),
         kInvalidUniqueId)
, mParentId(parentId)
, mLight(light)
, mLightId(sourceId)
, mPriority(priority)
, mLifeTime(lifeTime) {
  mLight.GetRadius();
  mLight.GetIntensity();
  SetLightPriorityAndId();
}

void CGameLight::SetLight(const CLight& light) {
  mLight = light;
  mLight.GetRadius();
  mLight.GetIntensity();
  SetLightPriorityAndId();
}

CLight CGameLight::GetLight() const {
  CLight ret = mLight;

  ret.SetPosition(GetTransform() * mLight.GetPosition());

  if (ret.GetType() != kLT_Point)
    ret.SetDirection(GetTransform().Rotate(mLight.GetDirection()).AsNormalized());

  return ret;
}

void CGameLight::Think(float dt, CStateManager& mgr) {
  if (mLifeTime > 0.f) {
    mLifeTime -= dt;

    if (mLifeTime <= 0.f)
      mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CGameLight)

void CGameLight::SetLightPriorityAndId() {
  mLight.SetPriority(mPriority);
  mLight.SetId(mLightId);
}

CGameLight::~CGameLight() {}
