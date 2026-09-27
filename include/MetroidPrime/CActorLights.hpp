#ifndef _CACTORLIGHTS
#define _CACTORLIGHTS

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGameArea;
class CStateManager;
class CAABox;

class CActorLights {
public:
  static const float kDefaultPositionUpdateThreshold;

  CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
               const int maxDynamicLights, const int maxAreaLights,
               float positionUpdateThreshold = kDefaultPositionUpdateThreshold,
               const bool ambientChannelOverflow = false, const bool useLightSet2 = false,
               const bool disableWorldLights = false);
  ~CActorLights();

  void BuildConstantAmbientLighting();
  void BuildConstantAmbientLighting(const CColor&);
  bool BuildAreaLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& bounds);
  void BuildDynamicLightList(const CStateManager& mgr, const CAABox& bounds);
  void BuildFakeLightList(const rstl::vector< CLight >&, const CColor&);
  void BuildFaceLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& aabb);

  void ActivateLights() const;
  uint GetActiveLightCount() const;
  uint GetActiveAreaLightCount() const { return mAreaLights.size(); }
  const CLight& GetLight(uint idx) const;

  bool GetNeedsRelight() const { return mDirty == TRUE; }
  bool HasShadowLight() const { return mShadowLightArrIdx != kInvalidShadowLightIndex; }
  const TAreaId GetAreaLightIndexForShadowLight() const { return mShadowLightArrIdx; }
  uint GetShadowLightArrIndex() const { return mShadowLightArrIdx; }
  int GetShadowLightIndex() const { return mShadowLightIdx; }
  CColor GetAmbientColor() const {
    return CColor(mAmbientColor.GetX(), mAmbientColor.GetY(), mAmbientColor.GetZ(), 1.f);
  }

  void SetAmbientColor(const CColor& color);
  void SetNeedsRelight(bool v) { mDirty = v; }
  void SetActorPositionBias(const CVector3f& bias) { mLightingPositionOffset = bias; }
  void SetCastShadows(bool v) { mCastShadows = v; }
  void SetFindShadowLight(bool v) { mFindShadowLight = v; }
  void SetShadowDynamicRangeThreshold(float t) { mShadowDynamicRangeThreshold = t; }

  void SetMaxAreaLights(int n) {
    mMaxAreaLights = n;
    mHasAreaLights = mMaxAreaLights > 0;
  }
  void SetInArea(bool v) { mInArea = v; }
  void SetMaxDynamicLights(int n) { mMaxDynamicLights = n; }
  void SetAmbienceGenerated(bool v) { mAmbienceGenerated = v; }
  void SetFindNearestDynamicLights(bool v) { mFindNearestDynamicLights = v; }
  bool GetIsDirty() const { return mDirty; }
  int GetMaxAreaLights() const { return mMaxAreaLights; }
  uint GetFramesBetweenRecalculation() const { return mAreaUpdateFramePeriod; }
  void SetFramesBetweenRecalculation(uint period) { mAreaUpdateFramePeriod = period; }

private:
  rstl::reserved_vector< CLight, 4 > mAreaLights;
  rstl::reserved_vector< CLight, 4 > mDynamicLights;
  CVector3f mAmbientColor;
  TAreaId mAid;
  bool mDirty : 1;
  bool mCastShadows : 1;
  bool mHasAreaLights : 1;
  bool mFindShadowLight : 1;
  bool mInArea : 1;
  bool mAmbienceGenerated : 1;
  bool mLayer2 : 1;
  bool mDisableWorldLights : 1;
  bool mInBrightLight : 1;
  bool mUseBrightLightLag : 1;
  bool mAmbientOnly : 1;
  bool mFindNearestDynamicLights;
  uint mShadowLightArrIdx;
  int mShadowLightIdx;
  uint mLastUpdateFrame;
  uint mAreaUpdateFramePeriod;
  CVector3f mLightingPositionOffset;
  int mMaxAreaLights;
  int mMaxDynamicLights;
  CVector3f mLastActorPos;
  float mActorPositionDeltaUpdateThreshold;
  float mShadowDynamicRangeThreshold;
  float mWorldLightingLevel;
  int mBrightLightIdx;
  uint mBrightLightLag;

  void UpdateBrightLight();
  void MultiplyLightingLevels(float level);
  void MoveAmbienceToLights(const CVector3f& color);
  void AddOverflowToLights(const CLight& light, const CVector3f& color, float mag);
  static void MergeOverflowLight(CLight& out, CVector3f& color, const CLight& in, float mag);

  static const int kInvalidShadowLightIndex;
  static int sFrameSchedulerCount;
};
CHECK_SIZEOF(CActorLights, 0x2e0)

#endif // _CACTORLIGHTS
