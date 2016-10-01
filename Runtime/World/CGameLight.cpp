#include "CGameLight.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"

namespace urde
{

CGameLight::CGameLight(TUniqueId uid, TAreaId aid, bool active, const std::string& name, const zeus::CTransform& xf,
                       TUniqueId parentId, const CLight& light, u32 w1, u32 w2, float f1)
: CActor(uid, active, name, CEntityInfo::CEntityInfo(aid, CEntity::NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(), kInvalidUniqueId),
  xe8_parentId(parentId), xec_light(light), x13c_(w1), x140_(w2), x144_(f1)
{
    xec_light.GetRadius();
    xec_light.GetIntensity();
    SetLightPriorityAndId();
}

void CGameLight::Think(float dt, CStateManager& mgr)
{
    if (x144_ <= 0.f)
        return;
    x144_ -= dt;

    if (x144_ <= 0.f)
        mgr.RemoveActor(GetUniqueId());
}

void CGameLight::SetLightPriorityAndId()
{
    xec_light.x3c_ = x140_;
    xec_light.x40_loadedIdx = x13c_;
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

TUniqueId CGameLight::GetParentId() const
{
    return xe8_parentId;
}
}
