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
    virtual float GetLeftLogicalThreshold() const=0;
    virtual float GetRightLogicalThreshold() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYER_HPP__
