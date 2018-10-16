#pragma once

namespace urde
{

class CGrappleParameters
{
    float x0_a;
    float x4_b;
    float x8_c;
    float xc_d;
    float x10_e;
    float x14_f;
    float x18_g;
    float x1c_h;
    float x20_i;
    float x24_j;
    float x28_k;
    bool x2c_lockSwingTurn;
public:
    CGrappleParameters(float a, float b, float c, float d,
                       float e, float f, float g, float h,
                       float i, float j, float k, bool lockSwingTurn)
    : x0_a(a), x4_b(b), x8_c(c), xc_d(d),
      x10_e(e), x14_f(f), x18_g(g), x1c_h(h),
      x20_i(i), x24_j(j), x28_k(k), x2c_lockSwingTurn(lockSwingTurn) {}

    bool GetLockSwingTurn() const { return x2c_lockSwingTurn; }
};

}

