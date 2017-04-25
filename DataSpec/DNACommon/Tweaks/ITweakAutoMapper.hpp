#ifndef __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
#define __DNACOMMON_ITWEAKAUTOMAPPER_HPP__

#include "ITweak.hpp"

namespace DataSpec
{
struct ITweakAutoMapper : public ITweak
{
    virtual bool GetShowOneMiniMapArea() const=0;
    virtual bool GetScaleMoveSpeedWithCamDist() const=0;
    virtual float GetCamDist() const=0;
    virtual float GetMinCamDist() const=0;
    virtual float GetMaxCamDist() const=0;
    virtual float GetMinCamRotateX() const=0;
    virtual float GetMaxCamRotateX() const=0;
    virtual float GetCamAngle() const=0;
    virtual float GetMiniCamDist() const=0;
    virtual float GetMiniCamXAngle() const=0;
    virtual float GetMiniCamAngle() const=0;
    virtual const zeus::CColor& GetSurfaceVisitedColor() const=0;
    virtual const zeus::CColor& GetOutlineVisitedColor() const=0;
    virtual const zeus::CColor& GetSurfaceUnvisitedColor() const=0;
    virtual const zeus::CColor& GetOutlineUnvisitedColor() const=0;
    virtual const zeus::CColor& GetSurfaceSelectVisitedColor() const=0;
    virtual const zeus::CColor& GetOutlineSelectVisitedColor() const=0;
    virtual float GetMapSurfaceNormColorLinear() const=0;
    virtual float GetMapSurfaceNormColorConstant() const=0;
    virtual float GetOpenMapScreenTime() const=0;
    virtual float GetCloseMapScreenTime() const=0;
    virtual float GetHintPanTime() const=0;
    virtual float GetCamZoomUnitsPerFrame() const=0;
    virtual float GetCamRotateDegreesPerFrame() const=0;
    virtual float GetBaseMapScreenCameraMoveSpeed() const=0;
    virtual const zeus::CColor& GetSurfaceSelectUnvisitedColor() const=0;
    virtual const zeus::CColor& GetOutlineSelectUnvisitedColor() const=0;
    virtual float GetMiniAlphaSurfaceVisited() const=0;
    virtual float GetAlphaSurfaceVisited() const=0;
    virtual float GetMiniAlphaOutlineVisited() const=0;
    virtual float GetAlphaOutlineVisited() const=0;
    virtual float GetMiniAlphaSurfaceUnvisited() const=0;
    virtual float GetAlphaSurfaceUnvisited() const=0;
    virtual float GetMiniAlphaOutlineUnvisited() const=0;
    virtual float GetAlphaOutlineUnvisited() const=0;
    virtual const zeus::CVector3f& GetDoorCenter() const=0;
    virtual float GetMiniMapViewportWidth() const=0;
    virtual float GetMiniMapViewportHeight() const=0;
    virtual float GetMiniMapCamDistScale() const=0;
    virtual float GetMapPlaneScaleX() const=0;
    virtual float GetMapPlaneScaleZ() const=0;
    virtual float GetUniverseCamDist() const=0;
    virtual float GetMinUniverseCamDist() const=0;
    virtual float GetMaxUniverseCamDist() const=0;
    virtual float GetSwitchToFromUniverseTime() const=0;
    virtual float GetCamPanUnitsPerFrame() const=0;
    virtual float GetCamVerticalOffset() const=0;
    virtual const zeus::CColor& GetMiniMapSamusModColor() const=0;
    virtual const zeus::CColor& GetAreaFlashPulseColor() const=0;
    virtual const zeus::CColor& GetDoorColor(int idx) const=0;
    virtual const zeus::CColor& GetOpenDoorColor() const=0;
};
}


#endif // __DNACOMMON_ITWEAKAUTOMAPPER_HPP__
