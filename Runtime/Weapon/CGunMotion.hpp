#ifndef __URDE_CGUNMOTION_HPP__
#define __URDE_CGUNMOTION_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"

namespace urde
{

namespace SamusGun
{
enum class EAnimationState
{
    Zero,
    One,
    Two
};
}

class CGunMotion
{
    CModelData x0_modelData;

public:
    CGunMotion(CAssetId, const zeus::CVector3f& vec);
    const CModelData& GetModelData() const { return x0_modelData; }
    void PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle, bool attack);
};

}

#endif // __URDE_CGUNMOTION_HPP__
