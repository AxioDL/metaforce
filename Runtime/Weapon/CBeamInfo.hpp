#pragma once

#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include <zeus/CColor.hpp>

namespace metaforce {
class CBeamInfo {
  u32 x0_;
  /*
   * 0x1: motion blur
   * 0x2: pulse effect
   * 0x4: one shot
   * 0x8: phazon damage
   */
  s32 x4_beamAttributes;
  CAssetId x8_contactFxId;
  CAssetId xc_pulseFxId;
  CAssetId x10_textureId;
  CAssetId x14_glowTextureId;
  s32 x18_length;
  float x1c_radius;
  float x20_expansionSpeed;
  float x24_lifeTime;
  float x28_pulseSpeed;
  float x2c_shutdownTime;
  float x30_contactFxScale;
  float x34_pulseFxScale;
  float x38_travelSpeed;
  zeus::CColor x3c_innerColor;
  zeus::CColor x40_outerColor;

public:
  explicit CBeamInfo(CInputStream& in)
  : x0_(in.ReadLong())
  , x4_beamAttributes(in.ReadLong())
  , x8_contactFxId(in.ReadLong())
  , xc_pulseFxId(in.ReadLong())
  , x10_textureId(in.ReadLong())
  , x14_glowTextureId(in.ReadLong())
  , x18_length(in.ReadFloat())
  , x1c_radius(in.ReadFloat())
  , x20_expansionSpeed(in.ReadFloat())
  , x24_lifeTime(in.ReadFloat())
  , x28_pulseSpeed(in.ReadFloat())
  , x2c_shutdownTime(in.ReadFloat())
  , x30_contactFxScale(in.ReadFloat())
  , x34_pulseFxScale(in.ReadFloat())
  , x38_travelSpeed(in.ReadFloat())
  , x3c_innerColor(in.Get<zeus::CColor>())
  , x40_outerColor(in.Get<zeus::CColor>()) {}

  CBeamInfo(s32 beamAttributes, CAssetId contactFxId, CAssetId pulseFxId, CAssetId textureId, CAssetId glowTextureId,
            s32 length, float radius, float expansionSpeed, float lifeTime, float pulseSpeed, float shutdownTime,
            float contactFxScale, float pulseFxScale, const zeus::CColor& innerColor, const zeus::CColor& outerColor,
            float travelSpeed)
  : x4_beamAttributes(beamAttributes)
  , x8_contactFxId(contactFxId)
  , xc_pulseFxId(pulseFxId)
  , x10_textureId(textureId)
  , x14_glowTextureId(glowTextureId)
  , x18_length(length)
  , x1c_radius(radius)
  , x20_expansionSpeed(expansionSpeed)
  , x24_lifeTime(lifeTime)
  , x28_pulseSpeed(pulseSpeed)
  , x2c_shutdownTime(shutdownTime)
  , x30_contactFxScale(contactFxScale)
  , x34_pulseFxScale(pulseFxScale)
  , x38_travelSpeed(travelSpeed)
  , x3c_innerColor(innerColor)
  , x40_outerColor(outerColor) {}

  s32 GetBeamAttributes() const { return x4_beamAttributes; }
  CAssetId GetContactFxId() const { return x8_contactFxId; }
  CAssetId GetPulseFxId() const { return xc_pulseFxId; }
  CAssetId GetTextureId() const { return x10_textureId; }
  CAssetId GetGlowTextureId() const { return x14_glowTextureId; }
  s32 GetLength() const { return x18_length; }
  float GetRadius() const { return x1c_radius; }
  float GetExpansionSpeed() const { return x20_expansionSpeed; }
  float GetLifeTime() const { return x24_lifeTime; }
  float GetPulseSpeed() const { return x28_pulseSpeed; }
  float GetShutdownTime() const { return x2c_shutdownTime; }
  float GetContactFxScale() const { return x30_contactFxScale; }
  float GetPulseFxScale() const { return x34_pulseFxScale; }
  float GetTravelSpeed() const { return x38_travelSpeed; }
  const zeus::CColor& GetInnerColor() const { return x3c_innerColor; }
  const zeus::CColor& GetOuterColor() const { return x40_outerColor; }
};

} // namespace metaforce
