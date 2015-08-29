#ifndef __DNACOMMON_ITWEAKPLAYER_HPP__
#define __DNACOMMON_ITWEAKPLAYER_HPP__

#include "../DNACommon.hpp"

namespace Retro
{

struct ITweakPlayer : BigYAML
{
    virtual float GetLeftLogicalThreshold() const=0;
    virtual float GetRightLogicalThreshold() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYER_HPP__
