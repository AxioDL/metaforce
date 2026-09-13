#ifndef _CFLAAHGRATENTACLE
#define _CFLAAHGRATENTACLE

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollisionActorManager;

class CFlaahgraTentacle : public CPatterned {
public:
  CFlaahgraTentacle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                    const CActorParameters& actParms);

  // CEntity
  ~CFlaahgraTentacle() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  bool Inside(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;

  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;

private:
  struct SSphereJointInfo {
    const char* name;
    float radius;
  };

  void AddSphereCollisionList(const SSphereJointInfo* sphereJoints, int jointCount,
                              rstl::vector< CJointCollisionDescription >& outJoints);
  void SetupCollisionManager(CStateManager& mgr);
  void ExtractTentacle(CStateManager& mgr);
  void RetractTentacle(CStateManager& mgr);
  void SaveBombSlotInfo(CStateManager& mgr);

  int x568_;
  rstl::single_ptr< CCollisionActorManager > x56c_collisionManager;
  float x570_;
  float x574_;
  float x578_;
  TUniqueId x57c_tentacleTipAct;
  CVector3f x580_forceVector;
  TUniqueId x58c_triggerId;
  bool x58e_24_ : 1;

  static const SSphereJointInfo skJointList[];
  static const char* const skpTentacleTip;
};
CHECK_SIZEOF(CFlaahgraTentacle, 0x590)

#endif // _CFLAAHGRATENTACLE
