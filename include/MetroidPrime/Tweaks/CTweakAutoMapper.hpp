#ifndef _CTWEAKAUTOMAPPER
#define _CTWEAKAUTOMAPPER

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "rstl/reserved_vector.hpp"

class CInputStream;
class CTweakAutoMapper;
class CTweakAutoMapper : public ITweakObject, public TOneStatic< CTweakAutoMapper > {
public:
  ~CTweakAutoMapper();
  CTweakAutoMapper(CInputStream&);

  bool mShowOneMiniMapArea : 1;
  bool x4_25_ : 1;
  bool mScaleMoveSpeedWithCamDist : 1;
  float mCamDist;
  float mMinCamDist;
  float mMaxCamDist;
  float mMinCamRotateX;
  float mMaxCamRotateX;
  float mCamAngle;
  float x20_;
  CColor mAutomapperWidgetColor;
  float mMiniCamDist;
  float mMiniCamXAngle;
  float mMiniCamAngle;
  float x34_;
  CColor mAutomapperWidgetMiniColor;
  CColor mSurfColorVisited;
  CColor mOutlineColorVisited;
  CColor mSurfColorUnvisited;
  CColor mOutlineColorUnvisited;
  CColor mSurfaceSelectColorVisited;
  CColor mOutlineSelectColorVisited;
  float mMapSurfaceNormColorLinear;
  float mMapSurfaceNormColorConstant;
  float x5c_;
  float x60_;
  float mOpenMapScreenTime;
  float mCloseMapScreenTime;
  float mHintPanTime;
  float mZoomUnitsPerFrame;
  float mRotateDegPerFrame;
  float mBaseMapScreenCameraMoveSpeed;
  CColor mSurfaceSelectColorUnvisited;
  CColor mOutlineSelectColorUnvisited;
  float mMiniAlphaSurfaceVisited;
  float mAlphaSurfaceVisited;
  float mMiniAlphaOutlineVisited;
  float mAlphaOutlineVisited;
  float mMiniAlphaSurfaceUnvisited;
  float mAlphaSurfaceUnvisited;
  float mMiniAlphaOutlineUnvisited;
  float mAlphaOutlineUnvisited;
  float mDoorCenterA;
  float mDoorCenterB;
  float mDoorCenterC;
  float xb0_;
  float xb4_;
  float mMiniMapViewportWidth;
  float mMiniMapViewportHeight;
  float mMiniMapCamDistScale;
  float mMapPlaneScaleX;
  float mMapPlaneScaleZ;
  bool xcc_;
  float mUniverseCamDist;
  float mMinUniverseCamDist;
  float mMaxUniverseCamDist;
  float mSwitchToFromUniverseTime;
  float mCamPanUnitsPerFrame;
  float mAutomapperScaleX;
  float mAutomapperScaleZ;
  float mCamVerticalOffset;
  CColor mMiniMapSamusModColor;
  CColor mAreaFlashPulseColor;
  CColor xf8_;
  CColor xfc_;
  rstl::reserved_vector< CColor, 5 > mDoorColors;
  CColor mDoorBorderColor;
  CColor mOpenDoorColor;
};
CHECK_SIZEOF(CTweakAutoMapper, 0x120)

extern CTweakAutoMapper* gpTweakAutoMapper;

#endif // _CTWEAKAUTOMAPPER
