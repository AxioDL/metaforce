#ifndef __URDE_CACTORLIGHTS_HPP__
#define __URDE_CACTORLIGHTS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"
#include "Graphics/CLight.hpp"

namespace urde
{
class CBooModel;
class CStateManager;
class CGameArea;

class CActorLights
{
    std::vector<CLight> x0_areaLights;
    std::vector<CLight> x144_dynamicLights;
    zeus::CColor x288_ambientColor;
    TAreaId x294_aid = kInvalidAreaId;

    union
    {
        struct
        {
            bool x298_24_ : 1;
            bool x298_25_ : 1;
            bool x298_26_ : 1;
            bool x298_27_ : 1;
            bool x298_28_inArea : 1;
            bool x298_29_ : 1;
            bool x298_30_ : 1;
            bool x298_31_ : 1;
            bool x299_24_ : 1;
            bool x299_25_overrideFirstDist : 1;
            bool x299_26_ : 1;
        };
        u16 _dummy = 0;
    };
    bool x29a_ = false;
    u32 x29c_ = -1;
    u32 x2a0_ = -1;
    u32 x2a4_ = 0;
    u32 x2a8_;
    zeus::CVector3f x2ac_;
    int x2b8_b;
    int x2bc_a;
    zeus::CVector3f x2c0_;
    float x2cc_;
    float x2d0_ = 0.f;
    float x2d4_ = 1.f;
    u32 x2d8_ = -1;
    u32 x2dc_overrideDist = 0;

public:
    CActorLights(u32, const zeus::CVector3f& vec, int, int, int, int, int, float);

    void BuildConstantAmbientLighting();
    void BuildConstantAmbientLighting(const zeus::CColor& color);
    void BuildFakeLightList(const std::vector<CLight>& lights, const zeus::CColor& color);
    void BuildFaceLightList(CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb);
    void BuildDynamicLightList(CStateManager& mgr, const zeus::CAABox& aabb);
    void MoveAmbienceToLights(const zeus::CVector3f& vec);
    void ActivateLights(CBooModel& model) const;

    void SetAmbientColor(const zeus::CColor& color) { x288_ambientColor = color; }
    const CLight& GetLight(u32 idx) const;
    u32 GetActiveLightCount() const;
};

}

#endif // __URDE_CACTORLIGHTS_HPP__
