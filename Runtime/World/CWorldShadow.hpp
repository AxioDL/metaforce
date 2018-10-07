#pragma once

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "Graphics/Shaders/CWorldShadowShader.hpp"

#define CWORLDSHADOW_FEEDBACK 0

namespace urde
{
class CStateManager;

class CWorldShadow
{
    CWorldShadowShader m_shader;
    zeus::CTransform x4_view;
    zeus::CTransform x34_model;
    float x64_objHalfExtent = 1.f;
    zeus::CVector3f x68_objPos = {0.f, 1.f, 0.f};
    zeus::CVector3f x74_lightPos;
    TAreaId x80_aid;
    s32 x84_lightIdx = -1;
    bool x88_blurReset = true;
#if CWORLDSHADOW_FEEDBACK
    std::experimental::optional<CTexturedQuadFilter> m_feedback;
#endif
public:
    CWorldShadow(u32 w, u32 h, bool rgba8);
    void EnableModelProjectedShadow(const zeus::CTransform& pos, s32 lightIdx, float f1);
    void DisableModelProjectedShadow();
    void BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, s32 lightIdx,
                                 const zeus::CAABox& aabb, bool motionBlur, bool lighten);
    void ResetBlur();
};

}

