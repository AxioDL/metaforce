#ifndef __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
#define __DNACOMMON_ITWEAKAUTOMAPPER_HPP__

#include "ITweak.hpp"

namespace DataSpec
{
struct ITweakAutoMapper : public ITweak
{
    virtual const zeus::CVector3f& GetDoorCenter() const=0;
    virtual float GetCamVerticalOffset() const=0;
};
}


#endif // __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
