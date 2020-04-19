#pragma once

#include "Runtime/MP1/World/CElitePirate.hpp"

namespace urde::MP1 {
class COmegaPirate : public CElitePirate {
private:
  class CFlash : public CActor {
  private:
    // TToken<CTexture> xe8_thermalSpotToken;
    // CTexture* xf0_thermalSpot = nullptr;
    float xf4_delay;
    float xf8_ = 0.f;
    float xfc_ = 0.f;

    CTexturedQuadFilter m_thermalSpotBlend;
    CTexturedQuadFilter m_thermalSpotSubtract;

  public:
    CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos, TLockedToken<CTexture>& thermalSpot,
           float delay);

    void Accept(IVisitor& visitor) override;
    void Think(float dt, CStateManager& mgr) override;
    void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
    void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
    void Render(CStateManager& mgr) override;
  };

  TUniqueId x990_launcherId2 = kInvalidUniqueId;
  enum class EFadeState {
    Zero,
    One,
    Two,
    Three,
  } x994_fadeState = EFadeState::Zero;
  float x998_ = 0.f;
  float x99c_alpha = 1.f;
  bool x9a0_visible = true;
  bool x9a1_fadeIn = true;
  std::vector<std::pair<TUniqueId, std::vector<TUniqueId>>> x9a4_scriptWaypointPlatforms;
  bool x9b4_ = false;
  std::vector<std::pair<TUniqueId, std::string_view>> x9b8_scriptEffects;
  enum class EScaleState {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
  } x9c8_scaleState = EScaleState::Zero;
  float x9cc_scaleTime = 0.f;
  zeus::CVector3f x9d0_initialScale;
  std::vector<std::pair<TUniqueId, std::string_view>> x9dc_scriptPlatforms;
  bool x9ec_ = false;
  CSkinnedModel x9f0_skeletonModel;
  float xa2c_skeletonAlpha = 0.f;
  enum class ESkeletonState {
    Zero,
    One,
    Two,
    Three,
  } xa30_skeletonState = ESkeletonState::Zero;
  float xa34_skeletonStateTime = 0.f;
  std::unique_ptr<CCollisionActorManager> xa38_collisionActorMgr1;
  bool xa3c_hearPlayer = false;
  pas::ELocomotionType xa40_locomotionType = pas::ELocomotionType::Relaxed;
  bool xa44_ = false;
  TUniqueId xa46_ = kInvalidUniqueId;
  TUniqueId xa48_ = kInvalidUniqueId;
  bool xa4a_ = false;
  zeus::CTransform xa4c_initialXf;
  enum class EXRayAlphaState {
    Zero,
    One,
    Two,
    Three,
  } xa7c_xrayAlphaState = EXRayAlphaState::Zero;
  float xa80_xrayAlpha = 1.f;
  float xa84_xrayAlphaStateTime = 0.f;
  bool xa88_ = false;
  float xa8c_ = 3.f;
  float xa90_ = 1.f;
  float xa94_ = 1.f;
  float xa98_maxEnergy = 0.f;
  std::unique_ptr<CCollisionActorManager> xa9c_collisionActorMgr2;
  std::vector<std::pair<TUniqueId, std::string_view>> xaa0_scriptSounds;
  float xab0_ = 0.f;
  std::vector<u32> xab4_;
  int xac4_ = 0;
  int xac8_ = 0;
  int xacc_ = 0;
  bool xad0_ = false;
  float xad4_cachedSpeed = 1.f;
  bool xad8_cover = false;
  TUniqueId xada_waypointId = kInvalidUniqueId;
  bool xadc_ = false;
  bool xadd_ = false;
  u8 xade_ = 0;
  bool xadf_launcher1FollowPlayer = true;
  bool xae0_launcher2FollowPlayer = true;
  CDamageVulnerability xae4_platformVuln = CDamageVulnerability::NormalVulnerabilty();
  int xb4c_ = 0;
  float xb50_ = 0.f;
  zeus::CColor xb54_platformColor = zeus::skWhite;
  float xb58_ = 2.5f;
  float xb5c_ = 0.f;
  float xb60_ = 0.f;
  float xb64_ = 17.f;
  int xb68_ = 0;
  bool xb6c_ = false;
  bool xb6d_ = false;
  bool xb6e_ = false;
  TLockedToken<CTexture> xb70_thermalSpot; // was TToken<CTexture>
  bool xb78_ = false;
  bool xb79_ = false;
  std::vector<u8> xb7c_;
  float xb8c_ = 0.f; // not initialized in ctr

public:
  COmegaPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, CElitePirateData data,
               CAssetId w1, CAssetId w2, CAssetId w3);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallDetach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dizzy(CStateManager& mgr, EStateMsg msg, float dt) override;

  bool Landed(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool CoverBlown(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  bool ShouldCallForBackup(CStateManager& mgr, float arg) override;
  bool HasWeakPointHead() const override { return false; }
  bool IsElitePirate() const override { return false; }
  void SetupHealthInfo(CStateManager& mgr) override;
  void SetLaunchersActive(CStateManager& mgr, bool val) override;
  SShockWaveData GetShockWaveData() const override {
    return {GetData().GetXF8(), GetData().GetXFC(), 24.78255f, GetData().GetX118(), GetData().GetX11C()};
  }

private:
  void CreateFlash(CStateManager& mgr, float arg);
  void SetupCollisionManager(CStateManager& mgr);
  void AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                              std::vector<CJointCollisionDescription>& outJoints) const;
  void AddOBBAutoSizeCollisionList(const SOBBJointInfo* joints, size_t count,
                                   std::vector<CJointCollisionDescription>& outJoints) const;
  void SetupCollisionActorInfo1(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr);
  void SetupCollisionActorInfo2(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr);
  void sub_8028cbec(u32 arg, CStateManager& mgr);
  u8 sub_8028c230() const;
  u8 sub_8028bfac() const;
  void TeleportToFurthestPlatform(CStateManager& mgr);
  void sub_8028f6f0(CStateManager& mgr, float dt);
  void sub_8028c704(CStateManager& mgr, float dt);
  void UpdateXRayAlpha(CStateManager& mgr, float dt);
  void UpdateScale(CStateManager& mgr, float dt);
  void UpdateSkeleton(CStateManager& mgr, float dt);
  void DeathDestroy(CStateManager& mgr);
  void sub_8028c840(u32 arg, CStateManager& mgr);
  zeus::CVector3f FindGround(const zeus::CVector3f& pos, CStateManager& mgr) const;
};
} // namespace urde::MP1
