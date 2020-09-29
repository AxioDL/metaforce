#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CLight.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CBooModel;
class CGameArea;
class CStateManager;

class CActorLights {
  static s32 sFrameSchedulerCount;
  std::vector<CLight> x0_areaLights;
  std::vector<CLight> x144_dynamicLights;
  zeus::CColor x288_ambientColor = zeus::skBlack;
  TAreaId x294_aid = kInvalidAreaId;
  bool x298_24_dirty : 1 = true;
  bool x298_25_castShadows : 1 = true;
  bool x298_26_hasAreaLights : 1 = false;
  bool x298_27_findShadowLight : 1 = false;
  bool x298_28_inArea : 1;
  bool x298_29_ambienceGenerated : 1;
  bool x298_30_layer2 : 1;
  bool x298_31_disableWorldLights : 1;
  bool x299_24_inBrightLight : 1 = true;
  bool x299_25_useBrightLightLag : 1 = false;
  bool x299_26_ambientOnly : 1 = false;
  bool x29a_findNearestDynamicLights = false;
  s32 x29c_shadowLightArrIdx = -1;
  s32 x2a0_shadowLightIdx = -1;
  u32 x2a4_lastUpdateFrame = 0;
  u32 x2a8_areaUpdateFramePeriod;
  zeus::CVector3f x2ac_actorPosBias;
  int x2b8_maxAreaLights;
  int x2bc_maxDynamicLights;
  zeus::CVector3f x2c0_lastActorPos;
  float x2cc_actorPositionDeltaUpdateThreshold;
  float x2d0_shadowDynamicRangeThreshold = 0.f;
  float x2d4_worldLightingLevel = 1.f;
  s32 x2d8_brightLightIdx = -1;
  u32 x2dc_brightLightLag = 0;

  static void MergeOverflowLight(CLight& out, zeus::CColor& color, const CLight& in, float colorMag);
  void AddOverflowToLights(const CLight& light, const zeus::CColor& color, float mag);
  void MoveAmbienceToLights(const zeus::CColor& color);
  void MultiplyLightingLevels(float level);
  void UpdateBrightLight();

public:
  CActorLights(u32 areaUpdateFramePeriod, const zeus::CVector3f& actorPosBias, int maxDynamicLights, int maxAreaLights,
               bool ambientChannelOverflow, bool layer2, bool disableWorldLights, float positionUpdateThreshold);

  void BuildConstantAmbientLighting();
  void BuildConstantAmbientLighting(const zeus::CColor& color);
  void BuildFakeLightList(const std::vector<CLight>& lights, const zeus::CColor& color);
  void BuildFaceLightList(const CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb);
  bool BuildAreaLightList(const CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb);
  void BuildDynamicLightList(const CStateManager& mgr, const zeus::CAABox& aabb);
  std::vector<CLight> BuildLightVector() const;
  void ActivateLights(CBooModel& model) const;
  void SetCastShadows(bool v) { x298_25_castShadows = v; }
  void SetHasAreaLights(bool v) { x298_26_hasAreaLights = v; }
  void SetFindShadowLight(bool v) { x298_27_findShadowLight = v; }
  void SetShadowDynamicRangeThreshold(float t) { x2d0_shadowDynamicRangeThreshold = t; }
  void SetAmbienceGenerated(bool v) { x298_29_ambienceGenerated = v; }
  void DisableAreaLights();
  void SetMaxAreaLights(int l) { x2b8_maxAreaLights = l; }
  void SetMaxDynamicLights(int l) { x2bc_maxDynamicLights = l; }
  void SetFindNearestDynamicLights(bool v) { x29a_findNearestDynamicLights = v; }
  void SetAmbientColor(const zeus::CColor& color) { x288_ambientColor = color; }
  const zeus::CColor& GetAmbientColor() const { return x288_ambientColor; }
  const CLight& GetLight(u32 idx) const;
  u32 GetActiveLightCount() const;
  int GetMaxAreaLights() const { return x2b8_maxAreaLights; }
  const std::vector<CLight>& GetAreaLights() const { return x0_areaLights; }
  const std::vector<CLight>& GetDynamicLights() const { return x144_dynamicLights; }
  bool GetIsDirty() const { return x298_24_dirty; }
  void SetDirty() { x298_24_dirty = true; }
  bool HasShadowLight() const { return x29c_shadowLightArrIdx != -1; }
  s32 GetShadowLightArrIndex() const { return x29c_shadowLightArrIdx; }
  s32 GetShadowLightIndex() const { return x2a0_shadowLightIdx; }
  u32 GetAreaUpdateFramePeriod() const { return x2a8_areaUpdateFramePeriod; }
  void SetAreaUpdateFramePeriod(u32 p) { x2a8_areaUpdateFramePeriod = p; }
  zeus::CVector3f GetActorPositionBias() const { return x2ac_actorPosBias; }
  void SetActorPositionBias(const zeus::CVector3f& bias) { x2ac_actorPosBias = bias; }
};

} // namespace urde
