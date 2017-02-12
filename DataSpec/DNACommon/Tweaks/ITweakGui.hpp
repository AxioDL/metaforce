#ifndef __DNACOMMON_ITWEAKGUI_HPP__
#define __DNACOMMON_ITWEAKGUI_HPP__

#include "ITweak.hpp"
#include "zeus/CVector2f.hpp"

namespace DataSpec
{

struct ITweakGui : ITweak
{
    enum class EHudVisMode : atUint32
    {
        Zero,
        One,
        Two,
        Three
    };

    enum class EHelmetVisMode : atUint32
    {
        Zero,
        One,
        Two,
        Three,
        Four
    };

    virtual EHudVisMode GetHudVisMode() const=0;
    virtual EHelmetVisMode GetHelmetVisMode() const=0;
    virtual atUint32 GetEnableAutoMapper() const=0;
    virtual atUint32 GetEnableTargetingManager() const=0;
    virtual atUint32 GetEnablePlayerVisor() const=0;
    virtual float GetScanAppearanceOffset() const=0;
    virtual float GetScanSpeed(int idx) const=0;
    virtual float GetXrayBlurScaleLinear() const=0;
    virtual float GetXrayBlurScaleQuadratic() const=0;
    virtual float GetWorldTransManagerCharsPerSfx() const=0;
};

}

#endif // __DNACOMMON_ITWEAKGUI_HPP__
