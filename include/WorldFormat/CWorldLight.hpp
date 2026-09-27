#ifndef _CWORLDLIGHT
#define _CWORLDLIGHT

#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CInputStream;
class CWorldLight {
  static const CVector3f kDefaultPosition;
  static const CVector3f kDefaultDirection;

public:
  enum EWorldLightType {
    kWLT_LocalAmbient,
    kWLT_Directional,
    kWLT_Custom,
    kWLT_Spot,
    kWLT_Spot2,
    kWLT_LocalAmbient2,
  };

  CWorldLight(CInputStream& in);

  CLight GetAsCGraphicsLight() const;
  bool DoesCastShadows() const { return mCastShadows; }
  const CVector3f& GetPosition() const { return mPosition; }

public:
  EWorldLightType mType;
  CVector3f mColor;
  CVector3f mPosition;
  CVector3f mDirection;
  float mQ;
  float mCutoffAngle;
  float x30_;
  bool mCastShadows;
  float x38_;
  EFalloffType mFalloff;
  float x40_;
};
CHECK_SIZEOF(CWorldLight, 0x44)

#endif // _CWORLDLIGHT
