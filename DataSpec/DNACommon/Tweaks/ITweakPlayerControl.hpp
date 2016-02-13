#ifndef __DNACOMMON_ITWEAKPLAYERCONTROL_HPP__
#define __DNACOMMON_ITWEAKPLAYERCONTROL_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{

struct ITweakPlayerControl : BigYAML
{
    virtual atUint32 GetMapping(atUint32) const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYERCONTROL_HPP__
