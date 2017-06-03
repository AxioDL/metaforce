#include "World/CGameLight.hpp"
#include "World/CActorParameters.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CGameLight::CGameLight(TUniqueId uid, TAreaId aid, bool active, const std::string& name, const zeus::CTransform& xf,
                       TUniqueId parentId, const CLight& light, u32 w1, u32 w2, float f1)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(), kInvalidUniqueId),
  xe8_parentId(parentId), xec_light(light), x13c_loadedIdx(w1), x140_priority(w2), x144_lifeTime(f1)
{
    xec_light.GetRadius();
    xec_light.GetIntensity();
    SetLightPriorityAndId();
}

void CGameLight::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

void CGameLight::Think(float dt, CStateManager& mgr)
{
    if (x144_lifeTime <= 0.f)
        return;
    x144_lifeTime -= dt;

    if (x144_lifeTime <= 0.f)
        mgr.FreeScriptObject(GetUniqueId());
}

void CGameLight::SetLightPriorityAndId()
{
    xec_light.x3c_priority = x140_priority;
    xec_light.x40_lightId = x13c_loadedIdx;
}

void CGameLight::SetLight(const CLight& light)
{
    xec_light = light;
    xec_light.GetRadius();
    xec_light.GetIntensity();
    SetLightPriorityAndId();
}

CLight CGameLight::GetLight() const
{
    CLight ret = xec_light;
    ret.SetPosition(x34_transform * xec_light.GetPosition());

    if (ret.GetType() != ELightType::Point)
        ret.SetDirection(x34_transform.rotate(xec_light.GetDirection()).normalized());

    return ret;
}
}
