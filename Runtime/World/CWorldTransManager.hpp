#ifndef __URDE_CWORLDTRANSMANAGER_HPP__
#define __URDE_CWORLDTRANSMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRandom16.hpp"

namespace urde
{

class CWorldTransManager
{
    float x0_ = 0.f;
    u32 x4_ = 0;
    u32 x8_ = 0;
    u8 x14_ = 0;
    float x18_;
    CRandom16 x20_ = CRandom16(99);
    u16 x24_ = 1189;
    u32 x28_ = 0;
    u8 x2c_ = 127;
    u8 x2d_ = 64;
    u32 x30_ = 0;
    float x38_ = 0.f;
    union
    {
        struct
        {
            bool x44_24_ : 1;
            bool x44_25_ : 1;
            bool x44_26_ : 1;
            bool x44_27_ : 1;
            bool x44_28_ : 1;
        };
        u8 dummy = 0;
    };
public:
    CWorldTransManager() : x44_24_(true) {}

    void DrawFirstPass() const {}
    void DrawSecondPass() const {}
    void DrawAllModels() const {}
    void UpdateLights(float) {}
    void UpdateEnabled(float) {}
    void UpdateDisabled(float) {}
    void Update(float) {}
    void DrawEnabled() const;
    void DrawDisabled() const;
    void sub_80209280() const {}
    void Draw() const
    {
        if (x30_ == 0)
            DrawDisabled();
        else if (x30_ == 1)
            DrawEnabled();
        else if (x30_ == 2)
            sub_80209280();
    }

    void StartTransition()
    {
        x0_ = 0.f;
        x18_ = 0.f;
        x44_24_ = false;
        x44_24_ = true;
    }

    void EndTransition() {}
    void PleaseStopSoon() {}
    bool IsTransitionEnabled() const { return false; }
    void DisableTransition() {}
    void TouchModels() {}
    void EnableTransition() {}
};

}

#endif // __URDE_CWORLDTRANSMANAGER_HPP__
