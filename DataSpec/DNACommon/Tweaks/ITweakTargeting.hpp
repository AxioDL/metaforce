#ifndef __DNACOMMON_ITWEAKTARGETING_HPP__
#define __DNACOMMON_ITWEAKTARGETING_HPP__

#include "ITweak.hpp"
namespace DataSpec
{
struct ITweakTargeting : public ITweak
{
    virtual float GetOvershootOffset() const =0;
    virtual float GetXD0() const = 0;
    virtual float GetScanTargetClampMin() const=0;
    virtual float GetScanTargetClampMax() const=0;
};
}

#endif // __DNACOMMON_ITWEAKTARGETING_HPP__
