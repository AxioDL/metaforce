#ifndef __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
#define __DNACOMMON_ITWEAKAUTOMAPPER_HPP__

#include "ITweak.hpp"

namespace DataSpec
{
struct ITweakAutoMapper : public ITweak
{
    virtual const zeus::CVector3f& GetDoorCenter() const=0;
    virtual float GetCamVerticalOffset() const=0;
    virtual float GetX28() const=0;
    virtual float GetX2C() const=0;
    virtual float GetX30() const=0;
    virtual float GetAlphaSurfaceVisited() const=0;
    virtual float GetAlphaOutlineVisited() const=0;
    virtual float GetAlphaSurfaceUnvisited() const=0;
    virtual float GetAlphaOutlineUnvisited() const=0;
    virtual float GetMiniMapViewportWidth() const=0;
    virtual float GetMiniMapViewportHeight() const=0;
};
}


#endif // __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
