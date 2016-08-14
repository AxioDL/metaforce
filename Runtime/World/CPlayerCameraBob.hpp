#ifndef __URDE_CPLAYERCAMERABOB_HPP__
#define __URDE_CPLAYERCAMERABOB_HPP__

#include <float.h>
#include "RetroTypes.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CPlayerCameraBob
{
public:
    enum class ECameraBobType
    {
        Zero,
        One
    };
private:
    ECameraBobType x0_type;
    zeus::CVector2f x4_vec;
    float xc_;
    float x10_ = 0.f;
    float x14_ = 0.f;
    float x18_ = 0.f;
    float x1c_ = 0.f;
    u32 x20_ = 8;
    u32 x24_ = 8;
    bool x28_ = false;
    bool x29_ = false;
    zeus::CTransform x2c_;
    float x5c_ = 0.f;
    float x60_ = 0.f;
    float x64_ = 0.f;
    float x68_ = 0.f;
    float x6c_ = 0.f;
    float x70_ = 0.f;
    float x74_ = 0.f;
    float x78_ = 0.f;
    zeus::CVector3f x7c_[4] = {zeus::CVector3f{0.f, 1.f, 0.f}};
    float xb0_[4] = {0.f};
    float xc4_ = 0.f;
    float xc8_ = 0.1f;
    u32 xcc_ = 0;
    zeus::CTransform xd0_;
    float x100_ = FLT_EPSILON;
    float x104_ = 0.f;
public:
    CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float);
};

}

#endif // __URDE_CPLAYERCAMERABOB_HPP__
