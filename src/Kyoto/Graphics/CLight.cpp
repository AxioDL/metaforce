#include <float.h>

#include "Kyoto/Graphics/CLight.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"

static const float gkEpsilon32 = FLT_EPSILON;

const CVector3f CLight::kDefaultPosition(0.f, 0.f, 0.f);
const CVector3f CLight::kDefaultDirection(0.f, -1.f, 0.f);

CLight::CLight(ELightType type, const CVector3f& position, const CVector3f& direction,
               const CColor& color, float cutoff)
: mPos(position)
, mDir(direction)
, mColor(color)
, mType(type)
, mSpotCutoff(cutoff)
, mDistC(0.f)
, mDistL(1.f)
, mDistQ(0.f)
, mAngleC(0.f)
, mAngleL(1.f)
, mAngleQ(0.f)
, mPriority(0)
, mLightId(0)
, mCachedRadius(0.f)
, mCachedIntensity(0.f)
, mIntensityDirty(true)
, mRadiusDirty(true) {}

CLight::CLight(const CVector3f& position, const CVector3f& direction, const CColor& color,
               float distC, float distL, float distQ, float angleC, float angleL, float angleQ)
: mPos(position)
, mDir(direction)
, mColor(color)
, mType(kLT_Custom)
, mSpotCutoff(0.f)
, mDistC(distC)
, mDistL(distL)
, mDistQ(distQ)
, mAngleC(angleC)
, mAngleL(angleL)
, mAngleQ(angleQ)
, mPriority(0)
, mLightId(0)
, mCachedRadius(0.f)
, mCachedIntensity(0.f)
, mIntensityDirty(true)
, mRadiusDirty(true) {}

CLight CLight::BuildLocalAmbient(const CVector3f& pos, const CColor& col) {
  return CLight(kLT_LocalAmbient, pos, kDefaultDirection, col, 180.f);
}


CLight CLight::BuildDirectional(const CVector3f& direction, const CColor& col) {
  return CLight(kLT_Directional, kDefaultPosition, direction, col, 180.f);
}


CLight CLight::BuildPoint(const CVector3f& pos, const CColor& color) {
  return CLight(kLT_Point, pos, kDefaultDirection, color, 180.f);
}

CLight CLight::BuildSpot(const CVector3f& pos, const CVector3f& dir, const CColor& color,
                         float cutoff) {
  return CLight(kLT_Spot, pos, dir, color, cutoff);
}

CLight CLight::BuildCustom(const CVector3f& pos, const CVector3f& dir, const CColor& color,
                           float distC, float distL, float distQ, float angleC, float angleL,
                           float angleQ) {
  return CLight(pos, dir, color, distC, distL, distQ, angleC, angleL, angleQ);
}


void CLight::SetAttenuation(float constant, float linear, float quadratic) {
  mDistC = constant;
  mDistL = linear;
  mDistQ = quadratic;
  mRadiusDirty = true;
  mIntensityDirty = true;
}


void CLight::SetAngleAttenuation(float constant, float linear, float quadratic) {
  mAngleC = constant;
  mAngleL = linear;
  mAngleQ = quadratic;
  mRadiusDirty = true;
  mIntensityDirty = true;
}

void CLight::SetColor(const CColor& col) {
  mColor = col;
  mRadiusDirty = true;
  mIntensityDirty = true;
}

void CLight::SetPosition(const CVector3f& position) { mPos = position; }

void CLight::SetDirection(const CVector3f& direction) { mDir = direction; }

float CLight::GetRadius() const {
  if (mRadiusDirty) {
    mCachedRadius = CalculateLightRadius();
    mRadiusDirty = false;
  }
  return mCachedRadius;
}


float CLight::CalculateLightRadius() const {
  if (mDistL < gkEpsilon32 && mDistQ < gkEpsilon32) {
    return 3.0E36f;
  }

  float intensity = GetIntensity();
  float ret = 0.f;
  if (mDistQ > gkEpsilon32) {
    const float mulVal = rstl::min_val(0.05882353f, 0.2f); // Yes, retro really did do this
    if (intensity > gkEpsilon32) {
      ret = CMath::SqrtF(intensity / (mulVal * mDistQ));
    }
  } else {
    const float mulVal = rstl::min_val(0.05882353f, 0.2f); // See above comment
    if (mDistL > gkEpsilon32) {
      ret = intensity / (mulVal * mDistL);
    }
  }

  return ret;
}

float CLight::GetIntensity() const {
  if (mIntensityDirty) {
    mIntensityDirty = false;
    float coef = 1.f;
    if (mType == kLT_Custom) {
      coef = mAngleC;
    }
    mCachedIntensity =
        coef *
        rstl::max_val(mColor.GetRed(), rstl::max_val(mColor.GetGreen(), mColor.GetBlue()));
  }
  return mCachedIntensity;
}
// Hack for float ordering
static void StrippedFunc() {
  static float f1 = -1.f;
  static float f2 = 0.f;
}

CVector3f CLight::GetNormalIndependentLightingAtPoint(const CVector3f& point) const {
  CVector3f floatCol(mColor.GetRed(), mColor.GetGreen(), mColor.GetBlue());
  if (mType == kLT_LocalAmbient)
    return floatCol;

  float dist = rstl::max_val((mPos - point).Magnitude(), gkEpsilon32);
  return floatCol / (dist * (mDistQ * dist) + (mDistL * dist + mDistC));
}
