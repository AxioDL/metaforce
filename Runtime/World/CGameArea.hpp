#pragma once

#include <array>

#include "Editor/ProjectResourceFactoryBase.hpp"
#include "Runtime/CObjectList.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CAreaOctTree.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CMetroidModelInstance.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CPVSAreaSet.hpp"
#include "Runtime/World/CEnvFxManager.hpp"
#include "Runtime/World/CPathFindArea.hpp"
#include "Runtime/World/CWorldLight.hpp"
#include "Runtime/World/IGameArea.hpp"

#include <hecl/ClientProcess.hpp>

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>

namespace urde {
class CStateManager;
class CScriptAreaAttributes;
struct SMREAHeader {
  u32 version = 0;
  zeus::CTransform xf;
  u32 modelCount;
  u32 secCount;
  u32 geomSecIdx;
  u32 sclySecIdx;
  u32 collisionSecIdx;
  u32 unkSecIdx;
  u32 lightSecIdx;
  u32 visiSecIdx;
  u32 pathSecIdx;
  u32 arotSecIdx;
};

class CDummyGameArea final : public IGameArea {
  friend class CDummyWorld;

  int x4_mlvlVersion;
  CAssetId x8_nameSTRG;
  CAssetId xc_mrea;
  TAreaId x10_areaId;
  zeus::CTransform x14_transform;
  std::vector<u16> x44_attachedAreaIndices;
  std::vector<Dock> x54_docks;

public:
  CDummyGameArea(CInputStream& in, int idx, int mlvlVersion);

  std::pair<std::unique_ptr<u8[]>, s32> IGetScriptingMemoryAlways() const override;
  TAreaId IGetAreaId() const override;
  CAssetId IGetAreaAssetId() const override;
  bool IIsActive() const override;
  TAreaId IGetAttachedAreaId(int) const override;
  u32 IGetNumAttachedAreas() const override;
  CAssetId IGetStringTableAssetId() const override;
  const zeus::CTransform& IGetTM() const override;
};

struct CAreaRenderOctTree {
  struct Node {
    u16 x0_bitmapIdx;
    u16 x2_flags;
    u16 x4_children[];

    u32 GetChildCount() const;
    zeus::CAABox GetNodeBounds(const zeus::CAABox& curAABB, int idx) const;

    void RecursiveBuildOverlaps(u32* out, const CAreaRenderOctTree& parent, const zeus::CAABox& curAABB,
                                const zeus::CAABox& testAABB) const;
  };

  const u8* x0_buf;
  u32 x8_bitmapCount;
  u32 xc_meshCount;
  u32 x10_nodeCount;
  u32 x14_bitmapWordCount;
  zeus::CAABox x18_aabb;
  const u32* x30_bitmaps;
  const u32* x34_indirectionTable;
  const u8* x38_entries;

  explicit CAreaRenderOctTree(const u8* buf);

  void FindOverlappingModels(std::vector<u32>& out, const zeus::CAABox& testAABB) const;
  void FindOverlappingModels(u32* out, const zeus::CAABox& testAABB) const;
};

class CGameArea final : public IGameArea {
  friend class CWorld;
  friend class CStateManager;

  TAreaId x4_selfIdx;
  CAssetId x8_nameSTRG;
  zeus::CTransform xc_transform;
  zeus::CTransform x3c_invTransform;
  zeus::CAABox x6c_aabb;
  CAssetId x84_mrea;
  s32 x88_areaId;
  std::vector<u16> x8c_attachedAreaIndices;
  std::vector<SObjectTag> x9c_deps1;
  std::vector<SObjectTag> xac_deps2;

  std::vector<u32> xbc_layerDepOffsets;
  std::vector<Dock> xcc_docks;
  std::vector<CToken> xdc_tokens;

  u32 xec_totalResourcesSize = 0;

  bool xf0_24_postConstructed : 1 = false;
  bool xf0_25_active : 1 = true;
  bool xf0_26_tokensReady : 1 = false;
  bool xf0_27_loadPaused : 1 = false;
  bool xf0_28_validated : 1 = false;

  enum class EPhase {
    LoadHeader,
    LoadSecSizes,
    ReserveSections,
    LoadDataSections,
    WaitForFinish
  } xf4_phase = EPhase::LoadHeader;

  std::list<std::shared_ptr<IDvdRequest>> xf8_loadTransactions;

public:
  class CChainIterator {
    CGameArea* m_area = nullptr;

  public:
    constexpr CChainIterator() = default;
    explicit constexpr CChainIterator(CGameArea* area) : m_area(area) {}
    CGameArea& operator*() const { return *m_area; }
    CGameArea* operator->() const { return m_area; }
    CChainIterator& operator++() {
      m_area = m_area->GetNext();
      return *this;
    }
    bool operator!=(const CChainIterator& other) const { return m_area != other.m_area; }
    bool operator==(const CChainIterator& other) const { return m_area == other.m_area; }
  };

  class CConstChainIterator {
    const CGameArea* m_area = nullptr;

  public:
    constexpr CConstChainIterator() = default;
    explicit constexpr CConstChainIterator(const CGameArea* area) : m_area(area) {}
    const CGameArea& operator*() const { return *m_area; }
    const CGameArea* operator->() const { return m_area; }
    CConstChainIterator& operator++() {
      m_area = m_area->GetNext();
      return *this;
    }
    bool operator!=(const CConstChainIterator& other) const { return m_area != other.m_area; }
    bool operator==(const CConstChainIterator& other) const { return m_area == other.m_area; }
  };

  class CAreaObjectList : public CObjectList {
  private:
    TAreaId x200c_areaIdx = 0;

  public:
    explicit CAreaObjectList(TAreaId areaIdx) : CObjectList(EGameObjectList::Invalid), x200c_areaIdx(areaIdx) {}

    bool IsQualified(const CEntity& ent) const override;
  };

  enum class EOcclusionState { Occluded, Visible };

  class CAreaFog {
    ERglFogMode x0_fogMode = ERglFogMode::None;
    zeus::CVector2f x4_rangeCur = {0.f, 1024.f};
    zeus::CVector2f xc_rangeTarget = {0.f, 1024.f};
    zeus::CVector2f x14_rangeDelta;
    zeus::CColor x1c_colorCur = {0.5f, 0.5f, 0.5f, 1.f};
    zeus::CColor x28_colorTarget = {0.5f, 0.5f, 0.5f, 1.f};
    float x34_colorDelta = 0.f;

  public:
    void SetCurrent() const;
    void Update(float dt);
    void RollFogOut(float rangeDelta, float colorDelta, const zeus::CColor& color);
    void FadeFog(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec1, float,
                 const zeus::CVector2f& vec2);
    void SetFogExplicit(ERglFogMode mode, const zeus::CColor& color, const zeus::CVector2f& range);
    bool IsFogDisabled() const;
    void DisableFog();
  };

  struct CPostConstructed {
    std::unique_ptr<CAreaOctTree> x0_collision;
    u32 x8_collisionSize = 0;
    std::optional<CAreaRenderOctTree> xc_octTree;
    std::vector<CMetroidModelInstance> x4c_insts;
    SShader m_materialSet{0};
    // std::unique_ptr<from unknown, pointless MREA section> x5c_;
    std::vector<CWorldLight> x60_lightsA;
    std::vector<CLight> x70_gfxLightsA;
    std::vector<CWorldLight> x80_lightsB;
    std::vector<CLight> x90_gfxLightsB;
    std::unique_ptr<CPVSAreaSet> xa0_pvs;
    u32 xa4_elemCount = 1024;
    struct MapEntry {
      s16 x0_id = -1;
      TUniqueId x4_uid = kInvalidUniqueId;
    };
    std::array<MapEntry, 1024> xa8_pvsEntityMap;
    u32 x10a8_pvsVersion = 0;
    TLockedToken<CPFArea> x10ac_pathToken;
    // bool x10b8_ = 0; optional flag for CToken
    CPFArea* x10bc_pathArea = nullptr;
    std::unique_ptr<CAreaObjectList> x10c0_areaObjs;
    std::unique_ptr<CAreaFog> x10c4_areaFog;
    const u8* x10c8_sclyBuf = nullptr;
    u32 x10d0_sclySize = 0;
    u32 x10d4_ = 0;
    const CScriptAreaAttributes* x10d8_areaAttributes = nullptr;
    EOcclusionState x10dc_occlusionState = EOcclusionState::Occluded;
    u32 x10e0_ = 0;
    float x10e4_occludedTime = 5.f;
    u32 x10e8_ = -1;
    u32 x10ec_ = 0;
    // std::vector<CAramToken> x10f0_tokens;
    u32 x1100_ = 0;
    u32 x1104_ = 0;
    bool x1108_24_ : 1 = false;
    bool x1108_25_modelsConstructed : 1 = false;
    bool x1108_26_ : 1 = false;
    bool x1108_27_ : 1 = false;
    bool x1108_28_occlusionPinged : 1 = false;
    bool x1108_29_pvsHasActors : 1 = false;
    bool x1108_30_ : 1 = false;
    std::vector<std::pair<const u8*, u32>> x110c_layerPtrs;
    float x111c_thermalCurrent = 0.f;
    float x1120_thermalSpeed = 0.f;
    float x1124_thermalTarget = 0.f;
    float x1128_worldLightingLevel = 1.f;
    float x112c_xraySpeed = 0.f;
    float x1130_xrayTarget = 1.f;
    float x1134_weaponWorldLightingSpeed = 0.f;
    float x1138_weaponWorldLightingTarget = 1.f;
    u32 x113c_playerActorsLoading = 0;

    CPostConstructed() = default;
  };

private:
  std::vector<std::pair<std::unique_ptr<u8[]>, int>> x110_mreaSecBufs;
  std::vector<std::pair<const u8*, int>> m_resolvedBufs;
  u32 x124_secCount = 0;
  u32 x128_mreaDataOffset = 0;
  std::unique_ptr<CPostConstructed> x12c_postConstructed;

  CGameArea* x130_next = nullptr;
  CGameArea* x134_prev = nullptr;
  EChain x138_curChain = EChain::ToDeallocate;

  void UpdateFog(float dt);
  void UpdateThermalVisor(float dt);
  void UpdateWeaponWorldLighting(float dt);

public:
  explicit CGameArea(CInputStream& in, int idx, int mlvlVersion);
  explicit CGameArea(CAssetId mreaId); // Warmup constructor
  ~CGameArea();

  bool IsFinishedOccluding() const;
  void ReadDependencyList();
  void SetLoadPauseState(bool paused);

  std::pair<std::unique_ptr<u8[]>, s32> IGetScriptingMemoryAlways() const override;
  TAreaId GetAreaId() const { return x4_selfIdx; }
  TAreaId IGetAreaId() const override { return x4_selfIdx; }
  CAssetId IGetAreaAssetId() const override { return x84_mrea; }
  bool IIsActive() const override;
  TAreaId IGetAttachedAreaId(int) const override;
  u32 IGetNumAttachedAreas() const override;
  CAssetId IGetStringTableAssetId() const override;
  const zeus::CTransform& IGetTM() const override;

  void SetXRaySpeedAndTarget(float speed, float target);
  void SetThermalSpeedAndTarget(float speed, float target);
  void SetWeaponWorldLighting(float speed, float target);

  CAssetId GetAreaAssetId() const { return x84_mrea; }
  const CAreaFog* GetAreaFog() const { return GetPostConstructed()->x10c4_areaFog.get(); }
  CAreaFog* GetAreaFog() { return GetPostConstructed()->x10c4_areaFog.get(); }
  float GetXRayFogDistance() const;
  EEnvFxType DoesAreaNeedEnvFx() const;
  bool DoesAreaNeedSkyNow() const;
  void OtherAreaOcclusionChanged();
  void PingOcclusionState();
  void PreRender();
  void AliveUpdate(float dt);
  void SetOcclusionState(EOcclusionState state);
  EOcclusionState GetOcclusionState() const { return GetPostConstructed()->x10dc_occlusionState; }
  void RemoveStaticGeometry();
  void AddStaticGeometry();
  // void TransferTokensToARAM();
  // void TransferARAMTokensOver();
  EChain SetChain(CGameArea* prev, EChain chain);
  bool StartStreamingMainArea();
  // void UnloadAllLoadedTextures();
  // void ReloadAllLoadedTextures();
  void ReloadAllUnloadedTextures();
  u32 GetNumPartSizes() const;
  void AllocNewAreaData(int, int);
  bool Invalidate(CStateManager* mgr);
  void KillmAreaData();
  void CullDeadAreaRequests();
  void StartStreamIn(CStateManager& mgr);
  void Validate(CStateManager& mgr);
  void LoadScriptObjects(CStateManager& mgr);
  std::pair<const u8*, u32> GetLayerScriptBuffer(int layer) const;
  void PostConstructArea();
  void FillInStaticGeometry(bool textures = true);
  void VerifyTokenList(CStateManager& stateMgr);
  void ClearTokenList();
  u32 GetPreConstructedSize() const;
  SMREAHeader VerifyHeader() const;
  TUniqueId LookupPVSUniqueID(TUniqueId id) const;
  s16 LookupPVSID(TUniqueId id) const;
  const CPVSAreaSet* GetAreaVisSet() const { return GetPostConstructed()->xa0_pvs.get(); }
  u32 Get1stPVSLightFeature(u32 lightIdx) const {
    return GetAreaVisSet() ? GetAreaVisSet()->Get1stLightIndex(lightIdx) : -1;
  }
  u32 Get2ndPVSLightFeature(u32 lightIdx) const {
    return GetAreaVisSet() ? GetAreaVisSet()->Get2ndLightIndex(lightIdx) : -1;
  }

  const zeus::CTransform& GetTransform() const { return xc_transform; }
  const zeus::CTransform& GetInverseTransform() const { return x3c_invTransform; }
  const zeus::CAABox& GetAABB() const { return x6c_aabb; }

  const std::vector<Dock>& GetDocks() const { return xcc_docks; }
  const Dock* GetDock(s32 dock) const { return &xcc_docks[dock]; }
  Dock* GetDock(s32 dock) { return &xcc_docks[dock]; }
  s32 GetDockCount() const { return xcc_docks.size(); }

  bool IsPostConstructed() const { return xf0_24_postConstructed; }
  CPostConstructed* GetPostConstructed() { return x12c_postConstructed.get(); }
  const CPostConstructed* GetPostConstructed() const { return x12c_postConstructed.get(); }

  bool IsValidated() const { return xf0_28_validated; }

  void SetAreaAttributes(const CScriptAreaAttributes* areaAttributes);
  bool GetActive() const { return xf0_25_active; }
  void SetActive(bool active) { xf0_25_active = active; }
  CObjectList* GetAreaObjects() const {
    return GetPostConstructed() ? GetPostConstructed()->x10c0_areaObjs.get() : nullptr;
  }

  CGameArea* GetNext() const { return x130_next; }

  static void WarmupShaders(const SObjectTag& mreaTag);

  s32 GetAreaSaveId() const { return x88_areaId; }
};

} // namespace urde
