#ifndef __DNACOMMON_ITWEAKGUI_HPP__
#define __DNACOMMON_ITWEAKGUI_HPP__

#include "../DNACommon.hpp"
#include "zeus/CVector2f.hpp"

namespace DataSpec
{

struct ITweakGui : BigYAML
{
    virtual float GetScanSpeed(int idx) const=0;
    virtual float GetXrayBlurScaleLinear() const=0;
    virtual float GetXrayBlurScaleQuadratic() const=0;
};

}

#endif // __DNACOMMON_ITWEAKGUI_HPP__
