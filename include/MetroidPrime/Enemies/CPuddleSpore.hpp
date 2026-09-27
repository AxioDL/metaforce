#ifndef _CPUDDLESPORE
#define _CPUDDLESPORE
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CPuddleSpore : public CPatterned {
public:
  CPuddleSpore(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
               const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
               const CPatternedInfo& pInfo, const EColliderType colType, const CAssetId glowFx,
               const float f1, const float f2, const float f3, const float f4, const float f5,
               const CActorParameters& actParms, const CAssetId weapon, const CDamageInfo& dInfo);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& point,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attrib) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  // CAi
  void KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& info,
                 float magnitude, bool direct, const bool inDeferred) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Run(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;

  const CAABox CalculateBoundingBox() const;

private:
  bool HitShell(const CVector3f& point) const;
  void KnockPlayer(CStateManager& mgr, float magnitude);
  void UpdateBoundingState(const CAABox& box, CStateManager& mgr, float dt);

  float mStateTime;
  float mGlowTime;
  float mGlowDuration;
  float mAttackDelay;
  float mTurnDelay;
  float x57c_;
  float mKnockPlayerImpulse;
  CVector3f mBodyOrigin;
  float mHalfExtent;
  float mHeight;
  float mHeightOffset;
  float mHeightScale;
  CCollidableAABox mCollisionPrimitive;
  int mCollisionState;
  int mAnimProgress;
  TLockedToken< CGenDescription > mGlowDescription;
  rstl::vector< CElementGen > mParticles;
  CProjectileInfo mProjectileInfo;
  bool mUpdateStateTime : 1;
  bool mUpdateGlowTime : 1;

  static int kEyeCount;
  static const char* kEyeLocators[];
};

CHECK_SIZEOF(CPuddleSpore, (VERSION >= VERSION_GM8E_02 ? 0x628 : 0x618))

#endif
