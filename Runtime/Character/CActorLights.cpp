#include "CActorLights.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{

s32 CActorLights::sFrameSchedulerCount = 0;
CActorLights::CActorLights(u32 unk, const zeus::CVector3f& vec, int a, int b, bool c, int d, int e, float f1)
: x2a8_(unk), x2ac_(vec), x2b8_b(b), x2bc_a(a), x2cc_(f1 * f1)
{
    x298_24_ = true;
    x298_25_ = true;

    x298_28_inArea = (!e && b > 0) ? true : false;
    x298_29_ = c;
    x298_30_ = d;
    x298_31_ = e;
    x299_24_ = true;

    sFrameSchedulerCount++;
    sFrameSchedulerCount &= 7;
}

void CActorLights::BuildConstantAmbientLighting()
{
    x299_26_ = true;
    x298_24_ = true;
    x29c_ = -1;
    x2a0_ = -1;
}

void CActorLights::BuildConstantAmbientLighting(const zeus::CColor& color)
{
    x299_26_ = false;
    x288_ambientColor = color;
    x294_aid = kInvalidAreaId;
    x298_24_ = true;
    x298_26_ = true;
    x29c_ = -1;
    x2a0_ = -1;
}

void CActorLights::BuildFakeLightList(const std::vector<CLight>& lights, const zeus::CColor& color)
{
    BuildConstantAmbientLighting(color);
    x144_dynamicLights = lights;
}

void CActorLights::BuildFaceLightList(CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb)
{
}

void CActorLights::BuildDynamicLightList(CStateManager& mgr, const zeus::CAABox& aabb)
{
}

void CActorLights::MoveAmbienceToLights(const zeus::CVector3f& vec)
{
}

void CActorLights::ActivateLights(CBooModel& model) const
{
    std::vector<CLight> lights;
    if (x298_28_inArea)
    {
        if (!x298_26_ || !x299_26_)
        {
            model.ActivateLights(lights);
            return;
        }
    }

    lights.push_back(CLight::BuildLocalAmbient(zeus::CVector3f::skZero, x288_ambientColor));

    if (x0_areaLights.size())
    {
        if (x2dc_overrideDist && x299_25_overrideFirstDist)
        {
            CLight overrideLight = x0_areaLights[0];
            overrideLight.SetColor(overrideLight.GetColor() * (1.f - x2dc_overrideDist / 15.f));
            lights.push_back(overrideLight);
        }
        else
            lights.push_back(x0_areaLights[0]);

        for (auto it = x0_areaLights.begin() + 1 ; it != x0_areaLights.end() ; ++it)
        {
            lights.push_back(*it);
        }
    }

    for (const CLight& light : x144_dynamicLights)
        lights.push_back(light);

    model.ActivateLights(lights);
}

const CLight& CActorLights::GetLight(u32 idx) const
{
    if (x298_28_inArea)
    {
        if (idx < x0_areaLights.size())
            return x0_areaLights[idx];
        return x144_dynamicLights[idx - x0_areaLights.size()];
    }
    return x144_dynamicLights[idx];
}

u32 CActorLights::GetActiveLightCount() const
{
    if (x298_28_inArea)
        return x0_areaLights.size() + x144_dynamicLights.size();
    return x144_dynamicLights.size();
}

}
