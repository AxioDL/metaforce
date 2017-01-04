#ifndef __URDE_CPROJECTEDSHADOW_HPP__
#define __URDE_CPROJECTEDSHADOW_HPP__

#include "Graphics/CTexture.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CStateManager;
class CModelData;
class CProjectedShadow
{
    CTexture x0_texture;
    zeus::CAABox x68_ = zeus::CAABox::skInvertedBox;
    bool x80_;
    bool x81_persistent;
    float x84_ = 1.f;
    zeus::CVector3f x88_ = zeus::CVector3f::skZero;
    float x94_zDistanceAdjust = 0.f;
    float x98_ = 1.f;

public:
    CProjectedShadow() = default;
    CProjectedShadow(u32, u32, bool);

    zeus::CAABox CalculateRenderBounds();
    void Render(CStateManager&, const CModelData&, const zeus::CTransform&, s32, const zeus::CVector3f&, float,
                   float);
};
}
#endif // __URDE_CPROJECTEDSHADOW_HPP__
