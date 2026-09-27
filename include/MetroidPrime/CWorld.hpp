#ifndef _CWORLD
#define _CWORLD

#include "Kyoto/SObjectTag.hpp"
#include "types.h"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CAudioGroupSet;
class CDvdRequest;
class CGameArea;
class CMapWorld;
class CModel;
class CRelay;
class CStateManager;
class IGameArea;
class CResFactory;

class IWorld {
public:
  virtual ~IWorld();
  virtual CAssetId IGetWorldAssetId() const = 0;
  virtual CAssetId IGetStringTableAssetId() const = 0;
  virtual CAssetId IGetSaveWorldAssetId() const = 0;
  virtual const CMapWorld* IGetMapWorld() const = 0;
  virtual CMapWorld* IMapWorld() = 0;
  virtual const IGameArea* IGetAreaAlways(TAreaId id) const = 0;
  virtual TAreaId IGetCurrentAreaId() const = 0;
  virtual TAreaId IGetAreaId(CAssetId id) const = 0;
  virtual bool ICheckWorldComplete() = 0;
  virtual rstl::string IGetDefaultAudioTrack() const = 0;
  virtual int IGetAreaCount() const = 0;
};

class CRelay {
public:
  explicit CRelay(CInputStream& in);
  const TEditorId& GetRelayId() const { return mRelay; }
  const TEditorId& GetTargetId() const { return mTarget; }
  const ushort& GetMessage() const { return mMsg; }
  bool GetActive() const { return mActive; }

private:
  TEditorId mRelay;
  TEditorId mTarget;
  ushort mMsg;
  bool mActive;
};

class CWorld final : public IWorld {
public:
  typedef rstl::vector< CRelay > CRelayList;

  struct CSoundGroupData {
    int mGroupId;
    CAssetId mAgscId;
    bool mLoadedIntoAram : 1;
    bool mLoaded : 1;
    rstl::string mName;
    rstl::optional_object< CToken > mGroupData;

  public:
    CSoundGroupData(int grpId, CAssetId agsc);
  };

  typedef rstl::vector< CSoundGroupData > CSoundGroupDataList;

  enum EChain {
    kC_Invalid = -1,
    kC_ToDeallocate,
    kC_Deallocated,
    kC_Loading,
    kC_Alive,
    kC_AliveJudgement,
  };

  enum EAreaTravelType { kATT_LoadAdjacent, kATT_SkipAdjacent };

  CWorld(IObjectStore& objStore, CResFactory& resFactory, CAssetId mlvlId);
  ~CWorld();
  CAssetId IGetWorldAssetId() const override;
  CAssetId IGetStringTableAssetId() const override;
  CAssetId IGetSaveWorldAssetId() const override;
  const CMapWorld* IGetMapWorld() const override;
  CMapWorld* IMapWorld() override;
  const IGameArea* IGetAreaAlways(TAreaId id) const override;
  TAreaId IGetCurrentAreaId() const override;
  TAreaId IGetAreaId(CAssetId id) const override;
  bool ICheckWorldComplete() override;
  rstl::string IGetDefaultAudioTrack() const override;
  int IGetAreaCount() const override;
  bool CheckWorldComplete(CStateManager* mgr, TAreaId aid, CAssetId mreaId);

  void SetLoadPauseState(bool);
  void CyclePauseState();
  void TouchSky() const;
  void DrawSky(const CTransform4f& xf) const;
  void StopSounds();
  void UnloadSoundGroups();
  bool ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr);
  void MoveToChain(CGameArea* area, EChain chain);
  void MoveAreaToChain3(TAreaId aid);
  void TravelToArea(const TAreaId& aid, CStateManager& mgr, EAreaTravelType travelType);
  void Update(float dt);
  void PreRender();
  CMapWorld* GetMapWorld() const;
  CMapWorld* MapWorld() { return GetMapWorld(); }
  void LoadSoundGroups();
  void LoadSoundGroup(uchar groupId, CAssetId agscId, CSoundGroupData& data);

  const CGameArea& GetAreaAlways(const TAreaId id) const { return *mAreas[id.Value()]; }
  CGameArea* Area(const TAreaId id) { return &*mAreas[id.Value()]; }
  const CGameArea* GetArea(const TAreaId id) const { return &*mAreas[id.Value()]; }
  bool IsAreaValid(const TAreaId id) const { return mAreas[id.Value()]->IsLoaded(); }
  bool DoesAreaExist(TAreaId id) const {
    return id.Value() >= 0 && id.Value() < mAreas.size();
  }
  CAssetId GetWorldAssetId() const { return mMlvlId; }
  TAreaId GetCurrentAreaId() const { return mCurAreaId; }
  TAreaId GetAreaIdForSaveId(uint saveId) const;
  TAreaId GetAreaId(CAssetId assetId) const;
  const CRelayList& GetRelays() const { return mRelays; }
  EEnvFxType GetNeededEnvFx() const { return mNeededFx; }

  static void PropogateAreaChain(CGameArea::EOcclusionState occlusionState, CGameArea* area,
                                 CWorld* world);

  CGameArea::CChainIterator ChainHead(EChain chain) const {
    return CGameArea::CChainIterator(mChainHeads[size_t(chain)]);
  }

  CGameArea::CConstChainIterator GetChainHead(EChain chain) const {
    return CGameArea::CConstChainIterator(mChainHeads[size_t(chain)]);
  }
  static CGameArea::CConstChainIterator GetAliveAreasEnd();
  static CGameArea::CChainIterator AliveAreasEnd();
  static CGameArea::CConstChainIterator skGlobalEnd;
  void StopGlobalSound(ushort soundId);
  void AddGlobalSound(ushort soundId, CSfxHandle handle);
  bool HasGlobalSound(ushort soundId) const;

  int GetNumAreas() const { return mAreas.size(); }
  bool AreSkyNeedsMet() const;

  void SetAreaAttributes(const TAreaId aid, CScriptAreaAttributes* attr) {
    Area(aid)->SetAreaAttributes(attr);
  }

private:
  static CGameArea::CChainIterator skGlobalNonConstEnd;

  enum Phase {
    kP_Loading,
    kP_LoadingMap,
    kP_LoadingMapAreas,
    kP_LoadingSkyBox,
    kP_LoadingSoundGroups,
    kP_Done,
  };

  Phase mPhase;
  CAssetId mMlvlId;
  CAssetId mStrgId;
  CAssetId mSavwId;
  rstl::vector< rstl::auto_ptr< CGameArea > > mAreas;
  CAssetId mMapwId;
  rstl::single_ptr< TCachedToken< CMapWorld > > mMapWorld;
  CRelayList mRelays;
  rstl::single_ptr< CDvdRequest > mLoadToken;
  rstl::single_ptr< char > mLoadBuf;
  uint mBufSz;
  rstl::reserved_vector< CGameArea*, 5 > mChainHeads;
  IObjectStore* mObjectStore;
  IFactory* mResFactory;
  TAreaId mCurAreaId;
  uint mLoadedAudioGrpCount;
  bool mCurrentAreaNeedsAllocation : 1;
  bool mLoadPaused : 1;
  bool mSkyboxActive : 1;
  bool mSkyboxVisible : 1;
  CSoundGroupDataList mSoundGroupData;
  rstl::string mDefAudioTrack;
  rstl::optional_object< TCachedToken< CModel > > mSkyboxWorld;
  rstl::optional_object< TLockedToken< CModel > > mSkyboxWorldLoaded;
  rstl::optional_object< TLockedToken< CModel > > mSkyboxOverride;
  EEnvFxType mNeededFx;
  rstl::reserved_vector< rstl::pair< ushort, CSfxHandle >, 10 > mGlobalSfxHandles;
};
CHECK_SIZEOF(CWorld, 0x11c)
NESTED_CHECK_SIZEOF(CWorld, CSoundGroupData, 0x28)

class CDummyWorld : public IWorld {
  enum Phase {
    kP_Loading,
    kP_LoadingMap,
    kP_LoadingMapAreas,
    kP_Done,
  };

  bool mLoadMap;
  Phase mPhase;
  CAssetId mMlvlId;
  CAssetId mStrgId;
  CAssetId mSavwId;
  rstl::vector< rstl::auto_ptr< CDummyGameArea > > mAreas;
  CAssetId mMapWorldId;
  rstl::single_ptr< TCachedToken< CMapWorld > > mMapWorld;
  rstl::single_ptr< CDvdRequest > mLoadToken;
  rstl::single_ptr< char > mLoadBuf;
  uint mBufSz;
  TAreaId mCurAreaId;

public:
  CDummyWorld(CAssetId mlvlId, const bool loadMap);
  ~CDummyWorld() override;
  CAssetId IGetWorldAssetId() const override;
  CAssetId IGetStringTableAssetId() const override;
  CAssetId IGetSaveWorldAssetId() const override;
  const CMapWorld* IGetMapWorld() const override;
  CMapWorld* IMapWorld() override;
  const IGameArea* IGetAreaAlways(TAreaId id) const override;
  TAreaId IGetCurrentAreaId() const override;
  TAreaId IGetAreaId(CAssetId id) const override;
  bool ICheckWorldComplete() override;
  rstl::string IGetDefaultAudioTrack() const override;
  int IGetAreaCount() const override;
};
CHECK_SIZEOF(CDummyWorld, 0x40)

#endif // _CWORLD
