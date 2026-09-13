#include "MetroidPrime/CGameLight.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CGameLight::CGameLight(TUniqueId uid, TAreaId aid, const bool active, const rstl::string& name,
                       const CTransform4f& xf, TUniqueId parentId, const CLight& light,
                       uint sourceId, uint priority, float lifeTime)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_parentId(parentId)
, xec_light(light)
, x13c_lightId(sourceId)
, x140_priority(priority)
, x144_lifeTime(lifeTime) {
  xec_light.GetRadius();
  xec_light.GetIntensity();
  SetLightPriorityAndId();
}

void CGameLight::SetLight(const CLight& light) {
  xec_light = light;
  xec_light.GetRadius();
  xec_light.GetIntensity();
  SetLightPriorityAndId();
}

CLight CGameLight::GetLight() const {
  CLight ret = xec_light;

  ret.SetPosition(GetTransform() * xec_light.GetPosition());

  if (ret.GetType() != kLT_Point)
    ret.SetDirection(GetTransform().Rotate(xec_light.GetDirection()).AsNormalized());

  return ret;
}

void CGameLight::Think(float dt, CStateManager& mgr) {
  if (x144_lifeTime > 0.f) {
    x144_lifeTime -= dt;

    if (x144_lifeTime <= 0.f)
      mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CGameLight)

void CGameLight::SetLightPriorityAndId() {
  xec_light.SetPriority(x140_priority);
  xec_light.SetId(x13c_lightId);
}

CGameLight::~CGameLight() {}
