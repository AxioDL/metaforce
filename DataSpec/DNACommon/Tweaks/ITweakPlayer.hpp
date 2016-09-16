#ifndef __DNACOMMON_ITWEAKPLAYER_HPP__
#define __DNACOMMON_ITWEAKPLAYER_HPP__

#include "../DNACommon.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec
{

struct ITweakPlayer : BigYAML
{
    virtual float GetPlayerHeight() const=0; // x26c
    virtual float GetPlayerXYHalfExtent() const=0; // x270
    virtual float GetPlayerSomething1() const=0; // x274
    virtual float GetPlayerSomething2() const=0; // x278
    virtual float GetPlayerSomething3() const=0; // x27c
    virtual float GetPlayerSomething4() const=0; // x134
    virtual bool GetPlayerSomething5() const=0; // x228_24
    virtual float GetPlayerSomething6() const=0; // x288
    virtual float GetPlayerSomething7() const=0; // x28c
    virtual float GetPlayerSomething8() const=0; // x290
    virtual float GetPlayerSomething9() const=0; // x294
    virtual float GetPlayerSomething10() const=0; // x298
    virtual float GetPlayerSomething11() const=0; // x29c
    virtual float GetPlayerSomething12() const=0; // x280
    virtual float GetPlayerSomething13() const=0; // x2b0
    virtual float GetPlayerSomething14() const=0; // x184
    virtual float GetPlayerSomething15() const=0; // x138
    virtual float GetPlayerSomething16() const=0; // x14c
    virtual float GetLeftLogicalThreshold() const=0;
    virtual float GetRightLogicalThreshold() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYER_HPP__
