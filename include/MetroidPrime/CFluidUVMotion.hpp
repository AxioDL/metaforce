#ifndef _CFLUIDUVMOTION
#define _CFLUIDUVMOTION

#include <rstl/reserved_vector.hpp>

class CFluidUVMotion {
public:
  enum EFluidMotion {
    kFM_Linear,
    kFM_Circular,
    kFM_Oscillate,
    kFM_NumLayers,
  };

  enum EFluidLayer {
    kFL_Color,
    kFL_Pattern1,
    kFL_Pattern2,
  };

  struct SFluidLayerMotion {
    EFluidMotion mMotion;
    float mOoTimeToWrap;
    float mOrientation;
    float mMagnitude;
    float mUvMul;
    float mUvScale;

    SFluidLayerMotion(EFluidMotion motion = kFM_Linear, float timeToWrap = 6.f,
                      float orientation = 0.f, float magnitude = 1.f, float uvMul = 5.f)
    : mMotion(motion)
    , mOoTimeToWrap(1.f / timeToWrap)
    , mOrientation(orientation)
    , mMagnitude(magnitude)
    , mUvMul(uvMul)
    , mUvScale(1.f / uvMul) {}
    SFluidLayerMotion(const SFluidLayerMotion& other)
    : mMotion(other.mMotion)
    , mOoTimeToWrap(other.mOoTimeToWrap)
    , mOrientation(other.mOrientation)
    , mMagnitude(other.mMagnitude)
    , mUvMul(other.mUvMul)
    , mUvScale(other.mUvScale) {}
  };

private:
  rstl::reserved_vector< SFluidLayerMotion, kFM_NumLayers > mFluidLayers;
  float mOoTimeToWrap;
  float mOrientation;

public:
  CFluidUVMotion(float timeToWrap, float orientation);
  CFluidUVMotion(float timeToWrap, float orientation, const SFluidLayerMotion& colorLayer,
                 const SFluidLayerMotion& pattern1Layer, const SFluidLayerMotion& pattern2Layer);
  //CFluidUVMotion(const CFluidUVMotion&);

  void CalculateFluidTextureOffset(float t, float offsets[kFM_NumLayers][2]) const;

  float GetOOTimeToWrapTexPage() const { return mOoTimeToWrap; }
  float GetOrientation() const { return mOrientation; }
  const SFluidLayerMotion& GetFluidLayerMotion(EFluidLayer layer) const {
    return mFluidLayers[layer];
  }
  const rstl::reserved_vector< SFluidLayerMotion, kFM_NumLayers >& GetFluidLayers() const {
    return mFluidLayers;
  }
};
CHECK_SIZEOF(CFluidUVMotion, 0x54)

#endif // _CFLUIDUVMOTION
