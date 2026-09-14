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
    return !x314_beamModelData.null() && !x314_beamModelData->IsNull();
  }
  bool HasSuitModelData() const {
    return !x318_suitModelData.null() &&
           (x318_suitModelData->HasAnimation() || x318_suitModelData->HasNormalModel());
  }
  void SetupEnvFx(CStateManager& mgr, bool set);
  TUniqueId GetNextPlayerActor() const { return x356_nextPlayerActor; }
  void SetNextPlayerActor(TUniqueId id) { x356_nextPlayerActor = id; }
  void SetIntoStateManager(CStateManager& mgr, bool set);

  CAnimRes x2e8_suitRes;
  CPlayerState::EBeamId x304_beam;
  CPlayerState::EPlayerSuit x308_suit;
  CPlayerState::EBeamId x30c_setBeamId;
  int x310_loadedCharIdx;
  rstl::single_ptr< CModelData > x314_beamModelData;
  rstl::single_ptr< CModelData > x318_suitModelData;
  rstl::single_ptr< TToken< CModel > > x31c_beamModel;
  rstl::single_ptr< TCachedToken< CModel > > x320_suitModel;
  rstl::single_ptr< TToken< CSkinRules > > x324_suitSkin;
  rstl::optional_object< TLockedToken< CSkinnedModel > > x328_backupModelData;
  rstl::optional_object< TCachedToken< CTexture > > x338_phazonIndirectTexture;
  int x348_deallocateBackupCountdown;
  float x34c_phazonOffsetAngle;
  uint x350_flags;
  bool x354_24_setBoundingBox : 1;
  bool x354_25_deferOnlineModelData : 1;
  bool x354_26_deferOfflineModelData : 1;
  bool x354_27_beamModelLoading : 1;
  bool x354_28_suitModelLoading : 1;
  bool x354_29_loading : 1;
  bool x354_30_enableLoading : 1;
  bool x354_31_deferOnlineLoad : 1;
  bool x355_24_areaTrackingLoad : 1;
  TUniqueId x356_nextPlayerActor;
};
CHECK_SIZEOF(CScriptPlayerActor, (VERSION >= VERSION_GM8P_00 ? 0x368 : 0x358))

#endif // _CSCRIPTPLAYERACTOR
