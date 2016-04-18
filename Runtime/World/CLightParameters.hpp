#ifndef __URDE_CLIGHTPARAMETERS_HPP__
#define __URDE_CLIGHTPARAMETERS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CLightParameters
{
    bool x4_a = false;
    float x8_b = 0.f;
    u32 xc_c = 0;
    float x10_d = 0.f;
    float x14_e = 0.f;
    zeus::CColor x18_f;
    bool x1c_g = false;
    bool x1d_h = false;
    u32 x20_i = 0;
    u32 x24_j = 1;
    u32 x28_k = 0;
    zeus::CVector3f x2c_l;
    u32 x38_m = 4;
    u32 x3c_n = 4;
public:
    CLightParameters() = default;
    CLightParameters(bool a, float b, u32 c, float d, float e, const zeus::CColor& f,
                     bool g, u32 i, u32 j, const zeus::CVector3f& l, u32 m, u32 n, bool h, u32 k)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e), x18_f(f), x1c_g(g), x1d_h(h), x20_i(i),
      x24_j(j), x28_k(k), x2c_l(l), x38_m(m), x3c_n(n)
    {
        if (x38_m > 4 || x38_m == -1)
            x38_m = 4;
        if (x3c_n > 4 || x3c_n == -1)
            x3c_n = 4;
    }
    static CLightParameters None() {return CLightParameters();}
};

}

#endif // __URDE_CLIGHTPARAMETERS_HPP__
