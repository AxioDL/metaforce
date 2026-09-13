#ifndef _CMETAREE
#define _CMETAREE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

class CPatternedInfo;
class CGenDescription;

class CMetaree : public CPatterned {
public:
  CMetaree(TUniqueId uid, const rstl::string& name, EFlavorType flavor, const CEntityInfo& info,
           const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
           const CDamageInfo& dInfo, float f1, const CVector3f& v1, float f2, EBodyType bodyType,
           float f3, float f4, const CActorParameters& aParms);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Touch(CActor&, CStateManager&) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Dead(CStateManager&, EStateMsg, float) override;
  void Halt(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void InActive(CStateManager&, EStateMsg, float) override;
  void Flee(CStateManager&, EStateMsg, float) override;
  void Explode(CStateManager&, EStateMsg, float) override;
  bool InRange(CStateManager&, float) override;
  bool Delay(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;

  // CPatterned
  void ThinkAboutMove(float) override;

private:
  float x568_delay;
  float x56c_haltDelay;
  float x570_dropHeight;
  CVector3f x574_offset;
  float x580_attackSpeed;
  CVector3f x584_lookPos;
  CVector3f x590_projectileDelta;
  CVector3f x59c_velocity;
  int x5a8_;
  CDamageInfo x5ac_damageInfo;
  ushort x5c8_attackSfx;
  bool x5ca_24_ : 1;
  bool x5ca_25_started : 1;
  bool x5ca_26_deactivated : 1;
  uint x5cc_;
};

#endif // _CMETAREE
