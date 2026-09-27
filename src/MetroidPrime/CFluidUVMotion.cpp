#include <MetroidPrime/CFluidUVMotion.hpp>

#include <Kyoto/Math/CMath.hpp>

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation)
: mFluidLayers(), mOoTimeToWrap(1.f / timeToWrap), mOrientation(orientation) {
  mFluidLayers.resize(kFM_NumLayers);
  mFluidLayers[0] = SFluidLayerMotion(CFluidUVMotion::kFM_Linear, 1000.f);
  mFluidLayers[1] = SFluidLayerMotion(CFluidUVMotion::kFM_Linear, 3.f);
  mFluidLayers[2] = SFluidLayerMotion(CFluidUVMotion::kFM_Linear, 5.f, CMath::Deg2Rad(45.f));
}

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation,
                               const SFluidLayerMotion& colorLayer,
                               const SFluidLayerMotion& pattern1Layer,
                               const SFluidLayerMotion& pattern2Layer)
: mFluidLayers(), mOoTimeToWrap(1.f / timeToWrap), mOrientation(orientation) {
  mFluidLayers.resize(kFM_NumLayers);
  mFluidLayers[0] = colorLayer;
  mFluidLayers[1] = pattern1Layer;
  mFluidLayers[2] = pattern2Layer;
}

void CFluidUVMotion::CalculateFluidTextureOffset(float t, float offsets[kFM_NumLayers][2]) const {
  float totalXOffset = t * GetOOTimeToWrapTexPage();
  float totalYOffset = CMath::FastCosR(GetOrientation()) * totalXOffset;
  totalXOffset *= CMath::FastSinR(GetOrientation());

  for (int i = 0; i < GetFluidLayers().size(); ++i) {
    const SFluidLayerMotion& layer = GetFluidLayerMotion(static_cast< EFluidLayer >(i));

    const float speedT = t * layer.mOoTimeToWrap;
    const float cycleT = speedT - floorf(speedT);
    float localY;
    float localX;
    switch (layer.mMotion) {
    case kFM_Linear:
      localY = 0.f;
      localX = speedT;
      break;
    case kFM_Circular:
      localY = layer.mMagnitude * CMath::FastSinR((M_PIF * 2) * cycleT);
      localX = layer.mMagnitude * CMath::FastCosR((M_PIF * 2) * cycleT);
      break;
    case kFM_Oscillate:
      localY = 0.f;
      localX = layer.mMagnitude * CMath::FastCosR((M_PIF * 2) * cycleT);
      break;
    default:
      localY = localX = 0.f;
      break;
    }

    const float x = totalXOffset + CMath::FastCosR(layer.mOrientation) * localY +
                    CMath::FastSinR(layer.mOrientation) * localX;
    const float y = totalYOffset + CMath::FastCosR(layer.mOrientation) * localX +
                    CMath::FastSinR(layer.mOrientation) * localY;
    offsets[i][0] = x - floorf(x);
    offsets[i][1] = y - floorf(y);
  }
}
