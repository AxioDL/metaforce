#ifndef _CBEAMINFO
#define _CBEAMINFO

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"

class CBeamInfo {
public:
  CBeamInfo(CInputStream& in);
  CBeamInfo(const int beamAttributes, const CAssetId contactFxId, const CAssetId pulseFxId,
            const CAssetId textureId, const CAssetId glowTextureId, const s32 length,
            const float radius, const float expansionSpeed, const float lifeTime,
            const float pulseSpeed, const float shutdownTime, const float contactFxScale,
            const float pulseFxScale, const CColor& innerColor, const CColor& outerColor,
            const float travelSpeed)
  : mBeamAttributes(beamAttributes)
  , mContactFxId(contactFxId)
  , mPulseFxId(pulseFxId)
  , mTextureId(textureId)
  , mGlowTextureId(glowTextureId)
  , mLength(length)
  , mRadius(radius)
  , mExpansionSpeed(expansionSpeed)
  , mLifeTime(lifeTime)
  , mPulseSpeed(pulseSpeed)
  , mShutdownTime(shutdownTime)
  , mContactFxScale(contactFxScale)
  , mPulseFxScale(pulseFxScale)
  , mTravelSpeed(travelSpeed)
  , mInnerColor(innerColor)
  , mOuterColor(outerColor) {}

  int GetBeamAttributes() const { return mBeamAttributes; }
  CAssetId GetContactFXId() const { return mContactFxId; }
  CAssetId GetPulseFXId() const { return mPulseFxId; }
  CAssetId GetTextureId() const { return mTextureId; }
  CAssetId GetGlowTextureId() const { return mGlowTextureId; }
  int GetLength() const { return mLength; }
  float GetRadius() const { return mRadius; }
  float GetExpansionSpeed() const { return mExpansionSpeed; }
  float GetLifeTime() const { return mLifeTime; }
  float GetPulseSpeed() const { return mPulseSpeed; }
  float GetShutdownTime() const { return mShutdownTime; }
  float GetContactFxScale() const { return mContactFxScale; }
  float GetPulseFxScale() const { return mPulseFxScale; }
  float GetTravelSpeed() const { return mTravelSpeed; }
  const CColor& GetInnerColor() const { return mInnerColor; }
  const CColor& GetOuterColor() const { return mOuterColor; }

private:
  uint x0_;
  /*
   * 0x1: motion blur
   * 0x2: pulse effect
   * 0x4: one shot
   * 0x8: phazon damage
   */
  int mBeamAttributes;
  CAssetId mContactFxId;
  CAssetId mPulseFxId;
  CAssetId mTextureId;
  CAssetId mGlowTextureId;
  int mLength;
  float mRadius;
  float mExpansionSpeed;
  float mLifeTime;
  float mPulseSpeed;
  float mShutdownTime;
  float mContactFxScale;
  float mPulseFxScale;
  float mTravelSpeed;
  CColor mInnerColor;
  CColor mOuterColor;
};
#endif // _CBEAMINFO
