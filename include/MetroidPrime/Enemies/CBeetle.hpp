#ifndef _CBEETLE
#define _CBEETLE

#include "types.h"

#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CTeamAiRole;

class CBeetle : public CPatterned {
public:
  enum EEntranceType {
    kET_FacePlayer = 0,
    kET_UseOrientation = 1,
  };

  CBeetle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
          const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
          CPatterned::EFlavorType flavor, CBeetle::EEntranceType entranceType,
          const CDamageInfo& touchDamage, const CDamageVulnerability& platingVuln,
          const CVector3f& tailAimReference, float initialAttackDelay, float retreatTime,
          float f3, const CDamageVulnerability& tailVuln, const CActorParameters& actorParams,
          const rstl::optional_object< CStaticRes >& tailModel);

  // CEntity
  ~CBeetle() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attrib) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void TakeDamage(const CVector3f& direction, float magnitude) override;
  bool IsListening() const override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  void FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;

  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDoubleSnap(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool ShouldJumpBack(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;
  bool NoPathNodes(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;

  // CPatterned
  void Burn(float duration, float damage) override;
  void Shock(float duration, float damage);
  CPathFindSearch* GetSearchPath() override;
  float GetGravityConstant() const override;

private:
  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  void RefinePathFindDest(CStateManager& mgr, CVector3f& dest);
  void SeparateFromMelees(CStateManager& mgr);
  void SetupRetreatPoints(CStateManager& mgr);
  s32 FindFurthestRetreatPoint(CStateManager& mgr);

  s32 mStateProg;
  EEntranceType mEntranceType;
  TUniqueId mAiMgr;
  ushort mPad0;
  CVector3f mTailAimReference;
  float mF3;
  CDamageInfo mTouchDamage;
  float mHeadbuttDist;
  float mJumpBackwardDist;
  float mAnimTimeRem;
  rstl::optional_object< CModelData > mTailModel;
  CPathFindSearch mPathFindSearch;
  rstl::reserved_vector< CVector3f, 8 > mRetreatPoints;
  CDamageVulnerability mPlatingVuln;
  CDamageVulnerability mTailVuln;
  float mAttackDelayTimer;
  float mStateFinishTimer;
  float mSpeedBackup;
  uint mPosDeviationCounter;
  CVector3f mPredictPos;
  float mIntoGroundFactor;
  float mRetreatTime;
  bool mHitSomething : 1;
  bool mBurrowing : 1;
  bool mCanSkid : 1;
  uchar mPad1[7];
};
CHECK_SIZEOF(CBeetle, (VERSION >= VERSION_GM8E_02 ? 0x850 : 0x840))

#endif // _CBEETLE
