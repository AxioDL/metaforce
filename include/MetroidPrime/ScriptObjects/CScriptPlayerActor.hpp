#ifndef _CSCRIPTPLAYERACTOR
#define _CSCRIPTPLAYERACTOR

#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CModel;
class CTexture;
class CSkinnedModel;
class CSkinRules;

class CScriptPlayerActor : public CScriptActor {
public:
  CScriptPlayerActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CAnimRes& animRes, const CModelData& mData,
                     const CAABox& aabb, bool setBoundingBox, const CMaterialList& matList,
                     float mass, float zMomentum, const CHealthInfo& hInfo,
                     const CDamageVulnerability& dVuln, const CActorParameters& aParams, bool loop,
                     bool active, uint flags, CPlayerState::EBeamId beam);

  // CEntity
  ~CScriptPlayerActor() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void SetActive(const bool active) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;

  void TouchModels(const CStateManager& mgr) const;

private:
  int GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const;
  int GetNextSuitCharIdx(const CStateManager& mgr) const;
  void LoadSuit(int charIdx);
  void LoadBeam(CPlayerState::EBeamId beam);
  void PumpBeamModel(CStateManager& mgr);
  void PumpSuitModel(CStateManager& mgr);
  void BuildBeamModelData();
  void SetupOfflineModelData();
  void SetupOnlineModelData();
  void TouchModels_Internal(const CStateManager& mgr) const;
  bool HasGunModelData() const {
    return !mBeamModelData.null() && !mBeamModelData->IsNull();
  }
  bool HasSuitModelData() const {
    return !mSuitModelData.null() &&
           (mSuitModelData->HasAnimation() || mSuitModelData->HasNormalModel());
  }
  void SetupEnvFx(CStateManager& mgr, bool set);
  TUniqueId GetNextPlayerActor() const { return mNextPlayerActor; }
  void SetNextPlayerActor(TUniqueId id) { mNextPlayerActor = id; }
  void SetIntoStateManager(CStateManager& mgr, bool set);

  CAnimRes mSuitRes;
  CPlayerState::EBeamId mBeam;
  CPlayerState::EPlayerSuit mSuit;
  CPlayerState::EBeamId mSetBeamId;
  int mLoadedCharIdx;
  rstl::single_ptr< CModelData > mBeamModelData;
  rstl::single_ptr< CModelData > mSuitModelData;
  rstl::single_ptr< TToken< CModel > > mBeamModel;
  rstl::single_ptr< TCachedToken< CModel > > mSuitModel;
  rstl::single_ptr< TToken< CSkinRules > > mSuitSkin;
  rstl::optional_object< TLockedToken< CSkinnedModel > > mBackupModelData;
  rstl::optional_object< TCachedToken< CTexture > > mPhazonIndirectTexture;
  int mDeallocateBackupCountdown;
  float mPhazonOffsetAngle;
  uint mFlags;
  bool mSetBoundingBox : 1;
  bool mDeferOnlineModelData : 1;
  bool mDeferOfflineModelData : 1;
  bool mBeamModelLoading : 1;
  bool mSuitModelLoading : 1;
  bool mLoading : 1;
  bool mEnableLoading : 1;
  bool mDeferOnlineLoad : 1;
  bool mAreaTrackingLoad : 1;
  TUniqueId mNextPlayerActor;
};
CHECK_SIZEOF(CScriptPlayerActor, (VERSION >= VERSION_GM8E_02 ? 0x368 : 0x358))

#endif // _CSCRIPTPLAYERACTOR
