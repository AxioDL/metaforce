#ifndef __PSHAG_CPLAYERSTATE_HPP__
#define __PSHAG_CPLAYERSTATE_HPP__

#include "RetroTypes.hpp"
#include "CBasics.hpp"
#include "CStaticInterference.hpp"
#include "IOStreams.hpp"
#include "rstl.hpp"

namespace urde
{

class CPlayerState
{
    static const u32 PowerUpMaxes[41];
    class CPowerUp
    {
        int x0_a = 0;
        int x4_b = 0;
    public:
        CPowerUp() {}
        CPowerUp(int a, int b) : x0_a(a), x4_b(b) {}
    };
    union
    {
        struct { bool x0_24_ : 1; bool x0_25_ : 1; bool x0_26_; };
        u32 dummy = 0;
    };

    u32 x4_ = 0;
    u32 x8_ = 0;
    float xc_baseHealth = 99.f;
    float x10_ = 50.f;
    u32 x14_ = 0;
    u32 x18_ = x14_;
    float x1c_ = 0.2f;
    u32 x20_ = 0;
    rstl::reserved_vector<CPowerUp, 41> x24_powerups;

    CStaticInterference x188_staticIntf;
public:
    CPlayerState() : x188_staticIntf(5) { x0_24_ = true; }
    CPlayerState(CBitStreamReader& stream);
};
}

#endif // __PSHAG_CPLAYERSTATE_HPP__
