#pragma once

#include "Runtime/MP1/World/CElitePirate.hpp"

namespace urde::MP1 {
class COmegaPirate : public CElitePirate {
private:
  class CFlash : public CActor {
  private:
    TToken<CTexture> xe8_thermalSpot;
    int xf0_ = 0;
    float xf4_;
    float xf8_ = 0.f;
    float xfc_ = 0.f;

  public:
    CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos, TToken<CTexture>& thermalSpot, float p5);

    void Accept(IVisitor& visitor) override;
  };

  TUniqueId x990_launcherId2 = kInvalidUniqueId;
  int x994_ = 0;
  float x998_ = 0.f;
  float x99c_ = 1.f;
  bool x9a0_ = true;
  bool x9a1_ = true;
  std::vector<std::pair<TUniqueId, std::vector<TUniqueId>>> x9a4_scriptWaypointPlatforms;
  bool x9b4_ = false;
  std::vector<std::pair<TUniqueId, std::string_view>> x9b8_scriptEffects;
  int x9c8_ = 0;
  float x9cc_ = 0.f;
  zeus::CVector3f x9d0_;
  std::vector<std::pair<TUniqueId, std::string_view>> x9dc_scriptPlatforms;
  bool x9ec_ = false;
  CSkinnedModel x9f0_;
  float xa2c_ = 0.f;
  int xa30_ = 0;
  float xa34_ = 0.f;
  std::unique_ptr<CCollisionActorManager> xa38_collisionActorMgr1;
  bool xa3c_hearPlayer = false;
  pas::ELocomotionType xa40_ = pas::ELocomotionType::Relaxed;
  bool xa44_ = false;
  TUniqueId xa46_ = kInvalidUniqueId;
  TUniqueId xa48_ = kInvalidUniqueId;
  bool xa4a_ = false;
  zeus::CTransform xa4c_initialXf;
  int xa7c_ = 0;
  float xa80_ = 1.f;
  float xa84_ = 0.f;
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
  TUniqueId xada_ = kInvalidUniqueId;
  bool xadc_ = false;
  bool xadd_ = false;
  u8 xade_ = 0;
  bool xadf_ = true;
  bool xae0_ = true;
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
  void sub_8028cd04(CStateManager& mgr, float dt);
  void sub_8028d7e4(CStateManager& mgr, float dt);
  void sub_8028d690(CStateManager& mgr, float dt);
  void Destroy(CStateManager& mgr);
  void sub_8028c840(u32 arg, CStateManager& mgr);
  zeus::CVector3f FindGround(const zeus::CVector3f& pos, CStateManager& mgr) const;
};
} // namespace urde::MP1
