#ifndef __URDE_CGAMEAREA_HPP__
#define __URDE_CGAMEAREA_HPP__

#include "zeus/CVector2f.hpp"
#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"

namespace urde
{

enum class ERglFogMode
{
    Four = 4
};

class CGameArea
{
public:
    class CAreaFog
    {
        zeus::CVector2f x4_ = {0.f, 1024.f};
        zeus::CVector2f xc_ = {0.f, 1024.f};
        zeus::CVector2f x14_;
        zeus::CVector3f x1c_ = {0.5f};
        zeus::CVector3f x28_ = {0.5f};
        float x34_ = 0.f;
    public:
        void SetFogExplicit(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec);
        void DisableFog();
        void Update(float dt);
    };
};

}

#endif // __URDE_CGAMEAREA_HPP__
