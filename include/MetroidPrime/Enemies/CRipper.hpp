#ifndef _CRIPPER
#define _CRIPPER

#include "MetroidPrime/CGrappleParameters.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

class CRipper : public CPatterned {
  static const uint skNumProperties;

public:
  static uint GetNumProperties() { return skNumProperties; }
  CRipper(TUniqueId uid, const rstl::string& name, EFlavorType type, const CEntityInfo& info,
          const CTransform4f& transform, const CModelData& modelData,
          const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
          const CGrappleParameters& grappleParams);
  ~CRipper();
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position, const CVector3f& direction, const CWeaponMode& mode, int projectileAtrib) const override;
  void KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& damage,
                 float mag, bool direct, const bool inDeferred) override;
  bool PathOver(CStateManager& mgr, float arg) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;

private:
  void ProcessGrapplePoint(CStateManager& mgr);
  void AddGrapplePoint(CStateManager& mgr);
  void RemoveGrapplePoint(CStateManager& mgr);
  void AddPlatform(CStateManager& mgr);
  void RemovePlatform(CStateManager& mgr);
  CGrappleParameters mGrappleParams;
  TUniqueId mGrapplePoint;
  TUniqueId mPlatform;
  bool mMuted : 1;
};

#endif // _CRIPPER
