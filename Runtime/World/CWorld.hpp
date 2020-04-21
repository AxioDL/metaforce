#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Audio/CAudioGroupSet.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/AutoMapper/CMapWorld.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/World/CEnvFxManager.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

namespace urde {
class CGameArea;
class CResFactory;
class IGameArea;
class IObjectStore;

class IWorld {
public:
  virtual ~IWorld() = default;
  virtual CAssetId IGetWorldAssetId() const = 0;
  virtual CAssetId IGetStringTableAssetId() const = 0;
  virtual CAssetId IGetSaveWorldAssetId() const = 0;
  virtual const CMapWorld* IGetMapWorld() const = 0;
  virtual CMapWorld* IGetMapWorld() = 0;
  virtual const IGameArea* IGetAreaAlways(TAreaId id) const = 0;
  virtual TAreaId IGetCurrentAreaId() const = 0;
  virtual TAreaId IGetAreaId(CAssetId id) const = 0;
  virtual bool ICheckWorldComplete() = 0;
  virtual std::string IGetDefaultAudioTrack() const = 0;
  virtual int IGetAreaCount() const = 0;
};

class CDummyWorld : public IWorld {
  enum class Phase {
    Loading,
    LoadingMap,
    LoadingMapAreas,
    Done,
  };

  bool x4_loadMap;
  Phase x8_phase = Phase::Loading;
  CAssetId xc_mlvlId;
  CAssetId x10_strgId;
  CAssetId x14_savwId;
  std::vector<CDummyGameArea> x18_areas;
  CAssetId x28_mapWorldId;
  TLockedToken<CMapWorld> x2c_mapWorld;
  std::shared_ptr<IDvdRequest> x30_loadToken;
  std::unique_ptr<uint8_t[]> x34_loadBuf;
  u32 x38_bufSz;
  TAreaId x3c_curAreaId = kInvalidAreaId;

public:
  CDummyWorld(CAssetId mlvlId, bool loadMap);
  ~CDummyWorld() override;
  CAssetId IGetWorldAssetId() const override;
  CAssetId IGetStringTableAssetId() const override;
  CAssetId IGetSaveWorldAssetId() const override;
  const CMapWorld* IGetMapWorld() const override;
  CMapWorld* IGetMapWorld() override;
  const IGameArea* IGetAreaAlways(TAreaId id) const override;
  TAreaId IGetCurrentAreaId() const override;
  TAreaId IGetAreaId(CAssetId id) const override;
  bool ICheckWorldComplete() override;
  std::string IGetDefaultAudioTrack() const override;
  int IGetAreaCount() const override;
};

class CWorld : public IWorld {
  friend class CStateManager;

public:
  class CRelay {
    TEditorId x0_relay = kInvalidEditorId;
    TEditorId x4_target = kInvalidEditorId;
    s16 x8_msg = -1;
    bool xa_active = false;

  public:
    CRelay() = default;
    explicit CRelay(CInputStream& in);

    TEditorId GetRelayId() const { return x0_relay; }
    TEditorId GetTargetId() const { return x4_target; }
    s16 GetMessage() const { return x8_msg; }
    bool GetActive() const { return xa_active; }

    static std::vector<CWorld::CRelay> ReadMemoryRelays(athena::io::MemoryReader& r);
  };

  struct CSoundGroupData {
    int x0_groupId;
    CAssetId x4_agscId;
    std::string xc_name;
    TCachedToken<CAudioGroupSet> x1c_groupData;

  public:
    CSoundGroupData(int grpId, CAssetId agsc);
  };

private:
  enum class Phase {
    Loading,
    LoadingMap,
    LoadingMapAreas,
    LoadingSkyBox,
    LoadingSoundGroups,
    Done,
  };

  Phase x4_phase = Phase::Loading;
  CAssetId x8_mlvlId;
  CAssetId xc_strgId;
  CAssetId x10_savwId;
  std::vector<std::unique_ptr<CGameArea>> x18_areas;
  CAssetId x24_mapwId;
  TLockedToken<CMapWorld> x28_mapWorld;
  std::vector<CRelay> x2c_relays;
  std::shared_ptr<IDvdRequest> x3c_loadToken;
  std::unique_ptr<uint8_t[]> x40_loadBuf;
  u32 x44_bufSz;
  u32 x48_chainCount = 0;
  std::array<CGameArea*, 5> x4c_chainHeads{};

  IObjectStore& x60_objectStore;
  IFactory& x64_resFactory;
  TAreaId x68_curAreaId = kInvalidAreaId;
  u32 x6c_loadedAudioGrpCount = 0;
  bool x70_24_currentAreaNeedsAllocation : 1 = true;
  bool x70_25_loadPaused : 1 = false;
  bool x70_26_skyboxActive : 1 = false;
  bool x70_27_skyboxVisible : 1 = false;
  std::vector<CSoundGroupData> x74_soundGroupData;
  std::string x84_defAudioTrack;
  std::optional<TLockedToken<CModel>> x94_skyboxWorld;
  std::optional<TLockedToken<CModel>> xa4_skyboxWorldLoaded;
  std::optional<TLockedToken<CModel>> xb4_skyboxOverride;
  EEnvFxType xc4_neededFx = EEnvFxType::None;
  rstl::reserved_vector<CSfxHandle, 10> xc8_globalSfxHandles;

  void LoadSoundGroup(int groupId, CAssetId agscId, CSoundGroupData& data);
  void LoadSoundGroups();
  void UnloadSoundGroups();
  void StopSounds();

public:
  void MoveToChain(CGameArea* area, EChain chain);
  void MoveAreaToAliveChain(TAreaId aid);
  bool CheckWorldComplete(CStateManager* mgr, TAreaId id, CAssetId mreaId);

  [[nodiscard]] auto GetChainHead(EChain chain) { return CGameArea::CChainIterator{x4c_chainHeads[size_t(chain)]}; }
  [[nodiscard]] auto GetChainHead(EChain chain) const {
    return CGameArea::CConstChainIterator{x4c_chainHeads[size_t(chain)]};
  }
  [[nodiscard]] auto begin() { return GetChainHead(EChain::Alive); }
  [[nodiscard]] auto end() { return AliveAreasEnd(); }
  [[nodiscard]] auto begin() const { return GetChainHead(EChain::Alive); }
  [[nodiscard]] auto end() const { return GetAliveAreasEnd(); }

  bool ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr);
  void TravelToArea(TAreaId aid, CStateManager& mgr, bool skipLoadOther);
  void SetLoadPauseState(bool paused);
  void CycleLoadPauseState();

  CWorld(IObjectStore& objStore, IFactory& resFactory, CAssetId mlvlId);
  ~CWorld() override;
  bool DoesAreaExist(TAreaId area) const;
  const std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() const { return x18_areas; }

  CMapWorld* GetMapWorld() { return x28_mapWorld.GetObj(); }
  const CMapWorld* GetMapWorld() const { return x28_mapWorld.GetObj(); }

  u32 GetRelayCount() const { return x2c_relays.size(); }
  CRelay GetRelay(u32 idx) const { return x2c_relays[idx]; }

  CAssetId IGetWorldAssetId() const override;
  CAssetId IGetStringTableAssetId() const override;
  CAssetId IGetSaveWorldAssetId() const override;
  const CMapWorld* IGetMapWorld() const override;
  CMapWorld* IGetMapWorld() override;
  const CGameArea* GetAreaAlways(TAreaId id) const;
  CGameArea* GetArea(TAreaId);
  s32 GetNumAreas() const { return x18_areas.size(); }
  const IGameArea* IGetAreaAlways(TAreaId id) const override;
  TAreaId IGetCurrentAreaId() const override;
  TAreaId GetCurrentAreaId() const { return x68_curAreaId; }
  TAreaId IGetAreaId(CAssetId id) const override;
  bool ICheckWorldComplete() override;
  std::string IGetDefaultAudioTrack() const override;
  int IGetAreaCount() const override;

  static void PropogateAreaChain(CGameArea::EOcclusionState occlusionState, CGameArea* area, CWorld* world);
  static constexpr CGameArea::CConstChainIterator GetAliveAreasEnd() { return CGameArea::CConstChainIterator{}; }
  static constexpr CGameArea::CChainIterator AliveAreasEnd() { return CGameArea::CChainIterator{}; }

  void Update(float dt);
  void PreRender();
  void TouchSky();
  void DrawSky(const zeus::CTransform& xf) const;
  void StopGlobalSound(u16 id);
  bool HasGlobalSound(u16 id) const;
  void AddGlobalSound(const CSfxHandle& hnd);
  EEnvFxType GetNeededEnvFx() const { return xc4_neededFx; }
  CAssetId GetWorldAssetId() const { return x8_mlvlId; }
  bool AreSkyNeedsMet() const;
  TAreaId GetAreaIdForSaveId(s32 saveId) const;
};

struct CWorldLayers {
  struct Area {
    u32 m_startNameIdx;
    u32 m_layerCount;
    u64 m_layerBits;
  };
  std::vector<Area> m_areas;
  std::vector<std::string> m_names;
  static void ReadWorldLayers(athena::io::MemoryReader& r, int version, CAssetId mlvlId);
};

} // namespace urde
