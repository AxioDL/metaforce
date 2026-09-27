#ifndef _CGAMEAREA
#define _CGAMEAREA

#include "types.h"

#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include "WorldFormat/CAreaRenderOctTree.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"
#include "WorldFormat/CWorldLight.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

struct TLayerId;

class CAreaOctTree;
class CAreaBspTree;
class CDvdRequest;
class CPVSAreaSet;
class CScriptAreaAttributes;
class CToken;

class IGameArea {
public:
  class Dock {
  public:
    struct SDockReference {
      TAreaId mArea;
      s16 mDock;
      short mLoadOther : 1;

      SDockReference(const TAreaId& area, short dock, short loadOther)
      : mArea(area), mDock(dock), mLoadOther(loadOther) {}
    };

  private:
    int mReferenceCount;
    rstl::vector< SDockReference > mDockReferences;
    rstl::reserved_vector< CVector3f, 4 > mPlaneVertices;
    bool mIsReferenced;

  public:
    const rstl::reserved_vector< CVector3f, 4 >& GetPlaneVertices() const {
      return mPlaneVertices;
    }
    int GetReferenceCount() const;
    const rstl::vector< SDockReference >& GetDockRefs() const { return mDockReferences; }
    Dock(CInputStream& in, const CTransform4f& xf);
    TAreaId GetConnectedAreaId(int other) const;
    int GetOtherDockNumber(int other) const;
    bool GetShouldLoadOther(int other) const;
    void SetShouldLoadOther(int other, bool should);
    bool ShouldLoadOtherArea(int other) const;
    const CVector3f& GetPoint(int idx) const { return mPlaneVertices[idx]; }
    bool IsReferenced() const;
    void SetReferenceCount(int v);
  };

  virtual ~IGameArea();
  virtual const CTransform4f& IGetTM() const = 0;
  virtual CAssetId IGetStringTableAssetId() const = 0;
  virtual uint IGetNumAttachedAreas() const = 0;
  virtual TAreaId IGetAttachedAreaId(int) const = 0;
  virtual bool IIsActive() const = 0;
  virtual CAssetId IGetAreaAssetId() const = 0;
  virtual int IGetAreaSaveId() const = 0;
  virtual rstl::pair< rstl::auto_ptr< char >, int > IGetScriptingMemoryAlways() const = 0;
};

class CPFArea;
class CGameArea : public IGameArea {
public:
  class CAreaFog {
  private:
    ERglFogMode mFogMode;
    CVector2f mRangeCur;
    CVector2f mRangeTarget;
    CVector2f mRangeDelta;
    CVector3f mColorCur;
    CVector3f mColorTarget;
    float mColorDelta;

  public:
    CAreaFog();
    void SetCurrent() const;
    void Update(float dt);
    void RollFogOut(float rangeDelta, float colorDelta, const CColor& color);
    void FadeFog(ERglFogMode, const CColor& color, const CVector2f& vec1, float,
                 const CVector2f& vec2);
    void SetFogExplicit(ERglFogMode mode, const CColor& color, const CVector2f& range);
    bool IsFogDisabled() const;
    void DisableFog();
  };

  class CChainIterator {
  protected:
    CGameArea* m_area;

  public:
    CChainIterator() : m_area(nullptr) {}
    explicit CChainIterator(CGameArea* area) : m_area(area) {}
    CGameArea& operator*() const { return *m_area; }
    CGameArea* operator->() const { return m_area; }
    CChainIterator& operator++() {
      m_area = m_area->GetNext();
      return *this;
    }
    bool operator!=(const CChainIterator& other) const { return other.m_area != m_area; }
    bool operator==(const CChainIterator& other) const { return m_area == other.m_area; }
  };

  class CConstChainIterator : protected CChainIterator {
  public:
    CConstChainIterator() {}
    explicit CConstChainIterator(const CGameArea* area)
    : CChainIterator(const_cast< CGameArea* >(area)) {}
    const CGameArea& operator*() const { return CChainIterator::operator*(); }
    const CGameArea* operator->() const { return CChainIterator::operator->(); }
    CConstChainIterator& operator++() {
      CChainIterator::operator++();
      return *this;
    }
    bool operator!=(const CConstChainIterator& other) const {
      return !CChainIterator::operator==(other);
    }
    bool operator==(const CConstChainIterator& other) const {
      return CChainIterator::operator==(other);
    }
  };

  enum EOcclusionState { kOS_Occluded, kOS_Visible };
  enum EARAMTransfer { kAT_Blocking, kAT_Async };

  class CAreaObjectList : public CObjectList {
  public:
    uchar IsQualified(const CEntity& ent) override;
    explicit CAreaObjectList(TAreaId areaId) : CObjectList(kOL_Invalid), mAreaId(areaId) {}

  private:
    TAreaId mAreaId;
  };

  struct SPVSActorInfo {
    ushort mPvsId;
    TUniqueId mUniqueId;

    SPVSActorInfo(ushort pvsId, TUniqueId uniqueId) : mPvsId(pvsId), mUniqueId(uniqueId) {}
  };

  struct CPostConstructed {
    rstl::auto_ptr< CAreaOctTree > mCollision;
    int mCollisionSize;
    rstl::optional_object< CAreaRenderOctTree > mOctTree;
    rstl::vector< CMetroidModelInstance > mInsts;
    rstl::single_ptr< CAreaBspTree > mBspTree;
    rstl::vector< CWorldLight > mLightsA;
    rstl::vector< CLight > mGfxLightsA;
    rstl::vector< CWorldLight > mLightsB;
    rstl::vector< CLight > mGfxLightsB;
    rstl::single_ptr< CPVSAreaSet > mPvs;
    rstl::reserved_vector< SPVSActorInfo, 1024 > mPvsEntityMap;
    int mPvsVersion;
    rstl::optional_object< TLockedToken< CPFArea > > mPathToken;
    CPFArea* mPathArea;
    rstl::single_ptr< CAreaObjectList > mAreaObjectList;
    rstl::single_ptr< CAreaFog > mAreaFog;
    rstl::auto_ptr< char > mSclyBuf;
    size_t mSclySize;
    const u8* mFirstMatPtr;
    const CScriptAreaAttributes* mAreaAttributes;
    EOcclusionState mOcclusionState;
    int x10e0_;
    float mOccludedTime;
    uint x10e8_;
    uint mFirstMatSection;
    rstl::vector< rstl::pair< CARAMToken, int > > mTokens;
    uint x1100_;
    uint x1104_;
    bool x1108_24_ : 1;
    bool mModelsConstructed : 1;
    bool x1108_26_ : 1;
    bool x1108_27_ : 1;
    bool mOcclusionPinged : 1;
    bool mPvsHasActors : 1;
    bool x1108_30_ : 1;
    rstl::vector< rstl::pair< int, int > > mLayerOffsets;
    float mThermalCurrent;
    float mThermalSpeed;
    float mThermalTarget;
    float mWorldLightingLevel;
    float mXraySpeed;
    float mXrayTarget;
    float mWeaponWorldLightingSpeed;
    float mWeaponWorldLightingTarget;
    int mPlayerActorsLoading;

    CPostConstructed();
  };

  CGameArea(CInputStream& in, int idx, const int mlvlVersion);
  ~CGameArea();
  static float skEntityThinkDisableDelayOnOcclusion;
  const CTransform4f& IGetTM() const override;
  CAssetId IGetStringTableAssetId() const override;
  uint IGetNumAttachedAreas() const override;
  TAreaId IGetAttachedAreaId(int) const override;
  bool IIsActive() const override;
  CAssetId IGetAreaAssetId() const override;
  int IGetAreaSaveId() const override;
  rstl::pair< rstl::auto_ptr< char >, int > IGetScriptingMemoryAlways() const override;

  TAreaId GetId() const { return mSelfIdx; }
  int GetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }
  TAreaId GetAttachedAreaId(int idx) const { return TAreaId(mAttachedAreaIndices[idx]); }
  int GetTokenCount() const { return mTokens.size(); }
  const rstl::pair< uint, uint >& GetAssetID(int idx) const { return mDeps2[idx]; }
  int GetScriptingSize() const {
    return mPostConstructedFlag ? mPostConstructed->mSclySize : 0;
  }
  const CTransform4f& GetTM() const { return mTransform; }
  const CTransform4f& GetInverseTransform() const { return mInvTransform; }
  bool IsLoaded() const { return mPostConstructedFlag; }
  bool IsActive() const { return mActive; }
  void SetActive(bool active) { mActive = active; }
  bool IsValidated() const { return mValidated; }
  const CAABox& GetAABB() const { return mAabb; }
  CGameArea* GetNext() const; // { return x130_next; }

  bool IsFinishedOccluding() const;
  void SetLoadPauseState(bool paused);
  void UpdateThermalVisor(float dt);
  void UpdateWeaponWorldLighting(float dt);
  TUniqueId LookupPVSUniqueID(TUniqueId id);
  uint LookupPVSID(TUniqueId id);
  void UpdateFog(const float dt);
  void SetXRaySpeedAndTarget(float speed, float target);
  void SetThermalSpeedAndTarget(float speed, float target);
  void SetWeaponWorldLighting(float speed, float target);

  float GetXRayFogDistance();
  rstl::pair< const uchar*, int > GetLayerScriptBuffer(const TLayerId layer) const;

  void SetAreaAttributes(CScriptAreaAttributes* areaAttributes);
  bool TryTakingOutOfARAM();

  bool StartStreamingMainArea();
  bool Invalidate(CStateManager* mgr);
  void Validate(CStateManager& mgr);
  void SetOcclusionState(EOcclusionState state);
  void RemoveStaticGeometry();
  void StartStreamIn(CStateManager& mgr);
  int SetChain(CGameArea* next, int chain);
  void AddStaticGeometry();
  bool TransferTokensToARAM();
  bool UnloadAllloadedTextures();
  bool ReloadAllUnloadedTextures();
  bool TransferARAMTokensOver(EARAMTransfer mode);
  void FillInStaticGeometry();
  void OtherAreaOcclusionChanged();
  void PingOcclusionState();
  void PreRender();
  void AliveUpdate(float dt);
  bool DoesAreaNeedSkyNow() const;
  EEnvFxType DoesAreaNeedEnvFx() const;

  CAssetId GetAreaAssetId() const { return mMrea; }
  const TAreaId& GetAreaId() const { return mSelfIdx; }
  int GetAreaSaveId() const { return mAreaId; }
  const Dock& GetDock(int idx) const { return mDocks[idx]; }
  Dock& DockNC(int idx) { return mDocks[idx]; }
  int GetDockCount() const { return mDocks.size(); }
  CAreaObjectList* ObjectList() const { return mPostConstructed->mAreaObjectList.get(); }
  const CAreaObjectList* GetObjectList() const {
    return mPostConstructed->mAreaObjectList.get();
  }
  const CAreaFog* GetAreaFog() const { return mPostConstructed->mAreaFog.get(); }
  CAreaFog* AreaFog() { return mPostConstructed->mAreaFog.get(); }
  EOcclusionState GetOcclusionState() const {
    if (!IsPostConstructed()) {
      return kOS_Occluded;
    }
    return mPostConstructed->mOcclusionState;
  }
  const rstl::vector< CWorldLight >& GetLightsA() const {
    return mPostConstructed->mLightsA;
  }
  const rstl::vector< CWorldLight >& GetLightsB() const {
    return mPostConstructed->mLightsB;
  }
  uint Get1stPVSLightFeature(uint idx) const;
  uint Get2ndPVSLightFeature(uint idx) const;
  const CPVSAreaSet* GetAreaVisSet() const { return mPostConstructed->mPvs.get(); }
  bool IsPostConstructed() const { return mPostConstructedFlag; }                         // name?
  CPostConstructed* GetPostConstructed() { return mPostConstructed.get(); }             // name?
  const CPostConstructed* GetPostConstructed() const { return mPostConstructed.get(); } // name?
  const CAreaOctTree& GetOctTree() const { return *GetPostConstructed()->mCollision; }
  CGameArea* GetNext() { return mNext; }                                                // name?
  CGameArea* GetPrev() { return mPrev; }                                                // name?
  int GetCurChain() const { return mCurChain; }                                         // name?

private:
  int GetPreConstructedSize() const;
  int GetPostConstructedSize() const;
  void ClearTokenList();
  void VerifyTokenList(CStateManager& mgr);
  void KillmAreaData();
  void LoadScriptObjects(CStateManager& mgr);
  void PostConstructArea();
  char* AllocNewAreaData(int offset, int size);
  void CullDeadAreaRequests();
  int VerifyHeader() const;
  int GetNumPartSizes() const;

  enum EPhase {
    kP_LoadHeader,
    kP_LoadSecSizes,
    kP_ReserveSections,
    kP_LoadDataSections,
    kP_WaitForFinish,
  };
  TAreaId mSelfIdx;
  CAssetId mNameSTRG;
  CTransform4f mTransform;
  CTransform4f mInvTransform;
  CAABox mAabb;
  CAssetId mMrea;
  int mAreaId;
  rstl::vector< ushort > mAttachedAreaIndices;
  rstl::vector< rstl::pair< uint, uint > > mDeps1;
  rstl::vector< rstl::pair< uint, uint > > mDeps2;
  rstl::vector< uint > mLayerDepOffsets;
  rstl::vector< Dock > mDocks;
  rstl::vector< CToken > mTokens;
  uint mTotalResourcesSize;
  bool mPostConstructedFlag : 1;
  bool mActive : 1;
  bool mTokensReady : 1;
  bool mLoadPaused : 1;
  bool mValidated : 1;
  EPhase mPhase;
  rstl::list< rstl::auto_ptr< CDvdRequest > > mLoadTransactions;
  rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > > mMreaSecBufs;
  int mUnk;
  int mSecCount;
  int mMreaDataOffset;
  rstl::single_ptr< CPostConstructed > mPostConstructed;
  CGameArea* mNext;
  CGameArea* mPrev;
  int mCurChain;
};
NESTED_CHECK_SIZEOF(CGameArea, CPostConstructed, 0x1140)
CHECK_SIZEOF(CGameArea, 0x13c)

class CDummyGameArea final : public IGameArea {
  friend class CDummyWorld;

public:
  CDummyGameArea(CInputStream& in, int idx, const int mlvlVersion);
  rstl::pair< rstl::auto_ptr< char >, int > IGetScriptingMemoryAlways() const override;
  int IGetAreaSaveId() const override;
  CAssetId IGetAreaAssetId() const override;
  bool IIsActive() const override;
  TAreaId IGetAttachedAreaId(int) const override;
  uint IGetNumAttachedAreas() const override;
  CAssetId IGetStringTableAssetId() const override;
  const CTransform4f& IGetTM() const override;

private:
  int mSelfIdx;
  CAssetId mNameSTRG;
  CAssetId mMrea;
  int mAreaId;
  CTransform4f mTransform;
  rstl::vector< u16 > mAttachedAreaIndices;
  rstl::vector< Dock > mDocks;
};
CHECK_SIZEOF(CDummyGameArea, 0x64)

#endif // _CGAMEAREA
