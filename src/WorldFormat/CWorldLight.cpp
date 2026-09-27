#include "WorldFormat/CWorldLight.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include <rstl/math.hpp>

const CVector3f CWorldLight::kDefaultPosition = CVector3f(0.f, 0.f, 0.f);
const CVector3f CWorldLight::kDefaultDirection = CVector3f(0.f, 1.f, 0.f);

CWorldLight::CWorldLight(CInputStream& in)
: mType(static_cast< EWorldLightType >(in.Get< uint >()))
, mColor(in)
, mPosition(in)
, mDirection(in)
, mQ(in.Get< float >())
, mCutoffAngle(in.Get< float >())
, x30_(in.Get< float >())
, mCastShadows(in.Get< bool >())
, x38_(in.Get< float >())
, mFalloff(static_cast< EFalloffType >(in.Get< uint >()))
, x40_(in.Get< float >()) {}

CLight CWorldLight::GetAsCGraphicsLight() const {
  CVector3f floatColor = mColor;
  CColor color(floatColor[0], floatColor[1], floatColor[2]);

  float q = mQ;
  if (mQ < FLT_EPSILON) {
    q = 10.f * FLT_EPSILON;
  }

  if (mType == kWLT_LocalAmbient) {
    floatColor *= q;

    floatColor[0] = rstl::min_val(1.f, floatColor[0]);
    floatColor[1] = rstl::min_val(1.f, floatColor[1]);
    floatColor[2] = rstl::min_val(1.f, floatColor[2]);
    CColor ambientColor(floatColor[0], floatColor[1], floatColor[2]);

    return CLight::BuildLocalAmbient(mPosition, ambientColor);
  }

  if (mType == kWLT_Directional) {
    return CLight::BuildDirectional(mDirection, color);
  }

  if (mType == kWLT_Spot) {
    CLight light =
        CLight::BuildSpot(mPosition, mDirection.AsNormalized(), color, mCutoffAngle / 2.f);
    float quadratic = mFalloff == kFT_Quadratic ? (25000.0f / q) : 0.f;
    float linear = mFalloff == kFT_Linear ? ((1.f / 0.004f) / q) : 0.f;
    float constant = mFalloff == kFT_Constant ? (2.0f / q) : 0.f;

    light.SetAttenuation(constant, linear, quadratic);
    return light;
  }

  float quadratic = mFalloff == kFT_Quadratic ? (25000.0f / q) : 0.f;
  float linear = mFalloff == kFT_Linear ? ((1.f / 0.004f) / q) : 0.f;
  float constant = mFalloff == kFT_Constant ? (2.0f / q) : 0.f;
  return CLight::BuildCustom(mPosition, CVector3f(1.f, 0.f, 0.f), color, constant, linear,
                             quadratic, 1.f, 0.f, 0.f);
}
