#ifndef _CTRYCLOPS
#define _CTRYCLOPS

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

class CTryclops : public CPatterned {
public:
  CTryclops(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, float suckForceMultiplier, float suckAngle,
            float suckRange, float launchSpeed);

  // CEntity
  ~CTryclops() override;
  DECLARE_TYPES_MATCH;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi and CPatterned
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  CPathFindSearch* GetSearchPath() override { return &x568_pathFindSearch; }
  bool IsListening() const override { return true; }
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetCover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dizzy(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool CoverBlown(CStateManager& mgr, float arg) override;
  bool Inside(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool IsDizzy(CStateManager& mgr, float arg) override;

private:
  static const CDamageVulnerability skPowerBombVulnerability;
  static CVector3f kBombPosOffset;
  static const char* const kMouthLctr;

  CPathFindSearch x568_pathFindSearch;
  CTransform4f x64c_playerRotation;
  float x67c_suckForceMultiplier;
  float x680_minSuckAngleProj;
  float x684_suckRange;
  float x688_launchSpeed;
  float x68c_ignoreMorphballTimer;
  uint x690_;
  TUniqueId x694_bombId;
  TUniqueId x696_;
  bool x698_24_shotTarget : 1;
  bool x698_25_targetingBomb : 1;
  bool x698_26_vulnerable : 1;
  bool x698_27_dizzy : 1;

  bool BallCloseToCollision(const CStateManager& mgr) const;
  void ShootPlayer(CStateManager& mgr, const CTransform4f& xf, float f);
  void ShootBomb(CStateManager& mgr, const CTransform4f& xf);
  void ApplySeparationBehavior(CStateManager& mgr);
  void SetBombPosition(CStateManager& mgr);
  void SetPlayerPosition(CStateManager& mgr, const CVector3f& locOrig);
  bool TargetCaught(const CVector3f& vec, float f);
  bool ObjectInVortexArea(const CVector3f& v1, const CVector3f& v2, const CAABox& box,
                          const CStateManager& mgr);
  void AttractPlayer(CStateManager& mgr, float f);
  void CenterPlayer(CStateManager& mgr, const CVector3f& dest, float f);
  void AttractBomb(CStateManager& mgr, float f);
};
CHECK_SIZEOF(CTryclops, 0x6A0)

#endif // _CTRYCLOPS
