#ifndef __DNACOMMON_ITWEAKPLAYERGUN_HPP__
#define __DNACOMMON_ITWEAKPLAYERGUN_HPP__

#include "../DNACommon.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec
{

struct ITweakPlayerGun : BigYAML
{
    virtual float GetSomething1() const=0; // x24
    virtual float GetSomething2() const=0; // x28
    virtual float GetSomething3() const=0; // x2c
    virtual float GetSomething4() const=0; // x30
    virtual float GetSomething5() const=0; // x34
    virtual float GetSomething6() const=0; // x38
};

}

#endif // __DNACOMMON_ITWEAKPLAYERGUN_HPP__
