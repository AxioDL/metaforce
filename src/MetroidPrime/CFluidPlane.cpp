#include "MetroidPrime/CFluidPlane.hpp"

#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/math.hpp"

float CFluidPlane::kRippleIntensityRange = 1.f;
const float gkFluidMaxCrest = 0.8f;

CFluidPlane::CFluidPlane(const CAssetId texPattern1, const CAssetId texPattern2,
                         const CAssetId texColor, const float alpha, const EFluidType fluidType,
                         const float rippleIntensity, const CFluidUVMotion& motion)
: mTexPattern1Id(texPattern1)
, mTexPattern2Id(texPattern2)
, mTexColorId(texColor)
, mAlpha(alpha)
, mFluidType(fluidType)
, mRippleIntensity(rippleIntensity)
, mUvMotion(motion) {
  if (gpResourceFactory->GetResourceTypeById(mTexPattern1Id) == FourCC('TXTR')) {
    mTexPattern1 = gpSimplePool->GetObj(SObjectTag(FourCC('TXTR'), mTexPattern1Id));
  }
  if (gpResourceFactory->GetResourceTypeById(mTexPattern2Id) == FourCC('TXTR')) {
    mTexPattern2 = gpSimplePool->GetObj(SObjectTag(FourCC('TXTR'), mTexPattern2Id));
  }
  if (gpResourceFactory->GetResourceTypeById(mTexColorId) == FourCC('TXTR')) {
    mTexColor = gpSimplePool->GetObj(SObjectTag(FourCC('TXTR'), mTexColorId));
  }
}

CFluidPlane::~CFluidPlane() {}

float CFluidPlane::CalculateRippleIntensity(float base) const {
  float mul;
  switch (mFluidType) {
  case kFT_NormalWater:
    mul = gpTweakGame->GetRippleIntensityNormal();
    break;
  case kFT_PoisonWater:
    mul = gpTweakGame->GetRippleIntensityPoison();
    break;
  case kFT_Lava:
    mul = gpTweakGame->GetRippleIntensityLava();
    break;
  case kFT_PhazonFluid:
  case kFT_Four:
    mul = 0.8f;
    break;
  default:
    mul = 1.f;
    break;
  }

  float intensityRange = GetRippleIntensityRange();
  float ret =
      base * mul * ((intensityRange * (1.f - mRippleIntensity) + 1.f) - (intensityRange * 0.5f));
  ret = rstl::min_val(rstl::max_val(ret, 0.f), 1.f);
  return ret;
}

float CFluidPlane::GetRippleScaleFromKineticEnergy(float baseI, float velDot) {
  const float energy = 0.5f * baseI * velDot * velDot;
  float tmp = CMath::FastSqrtF(energy);
  return tmp >= 160.f ? 1.f : tmp * (1.f / 160.f);
}

void CFluidPlane::AddRipple(const float mag, const TUniqueId rippler, const CVector3f& center,
                            const CScriptWater& water, CStateManager& mgr) {
  if (!water.CanRippleAtPoint(center)) {
    return;
  }
  const float intensity = CalculateRippleIntensity(mag);
  mgr.FluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, intensity));
}

void CFluidPlane::AddRipple(const float mag, const TUniqueId rippler, const CVector3f& center,
                            const CVector3f& velocity, const CScriptWater& water,
                            CStateManager& mgr, const CVector3f& upVec) {
  if (!water.CanRippleAtPoint(center)) {
    return;
  }

  float intensity = CalculateRippleIntensity(
      GetRippleScaleFromKineticEnergy(mag, CVector3f::Dot(upVec, velocity)));
  mgr.FluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, intensity));
}

void CFluidPlane::AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr) {
  if (!water.CanRippleAtPoint(ripple.GetCenter())) {
    return;
  }

  mgr.FluidPlaneManager()->RippleManager().AddRipple(ripple);
}

void CFluidPlane::Render(const CStateManager& mgr, const CAABox&, const CFrustumPlanes&,
                         const CRippleManager&, const CVector3f&) {}
