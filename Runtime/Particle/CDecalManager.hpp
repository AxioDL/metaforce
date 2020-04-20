#pragma once

#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Particle/CDecal.hpp"
#include <zeus/CFrustum.hpp>

namespace urde {
class CStateManager;

class CDecalManager {
  struct SDecal {
    std::optional<CDecal> x0_decal;
    TAreaId x70_areaId;
    s8 x74_index;
    bool x75_24_notIce : 1;
    SDecal(const std::optional<CDecal>& decal, TAreaId aid, s8 idx, bool notIce)
    : x0_decal(decal), x70_areaId(aid), x74_index(idx), x75_24_notIce(notIce) {}
  };

  static bool m_PoolInitialized;
  static s32 m_FreeIndex;
  static float m_DeltaTimeSinceLastDecalCreation;
  static s32 m_LastDecalCreatedIndex;
  static CAssetId m_LastDecalCreatedAssetId;
  static rstl::reserved_vector<SDecal, 64> m_DecalPool;
  static rstl::reserved_vector<s32, 64> m_ActiveIndexList;
  static rstl::reserved_vector<s32, 64>::iterator RemoveFromActiveList(rstl::reserved_vector<s32, 64>::iterator it,
                                                                       s32 idx);

public:
  static void Initialize();
  static void Reinitialize();
  static void Shutdown();
  static void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr);
  static void Update(float dt, CStateManager& mgr);
  static void AddDecal(const TToken<CDecalDescription>& decal, const zeus::CTransform& xf, bool notIce,
                       CStateManager& mgr);
};

} // namespace urde
