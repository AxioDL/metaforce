#ifndef _CFLAAHGRAPLANTS
#define _CFLAAHGRAPLANTS
#include "types.h"

#include "Collision/COBBox.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

class CPatternedInfo;
class CGenDescription;

class CFlaahgraPlants : public CActor {
public:
  CFlaahgraPlants(const TToken< CGenDescription >&, const CActorParameters&, TUniqueId, TAreaId,
                  TUniqueId, const CTransform4f&, const CDamageInfo&, const CVector3f&);

  // CEntity
  ~CFlaahgraPlants() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  rstl::auto_ptr< CElementGen > mElementGen;
  TUniqueId mOwnerId;
  CDamageInfo mDamageInfo;
  rstl::optional_object< CAABox > mAabox;
  float mLastDt;
  COBBox mObbox;
  TUniqueId mColAct;
};

#endif // _CFLAAHGRAPLANTS
