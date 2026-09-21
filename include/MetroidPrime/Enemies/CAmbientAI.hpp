#ifndef _CAMBIENTAI
#define _CAMBIENTAI

#include "MetroidPrime/CPhysicsActor.hpp"

#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"

class CAmbientAI : public CPhysicsActor {
public:
  CAmbientAI(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& mData, const CAABox& aabox,
             const CMaterialList& materialList, float mass, const CHealthInfo& hInfo,
             const CDamageVulnerability& dVuln, const CActorParameters& aParms, float alertRange,
             float impactRange, int alertAnim, int impactAnim, bool Active);

  /* CEntity */
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  /* CActor */
  CHealthInfo* HealthInfo(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;

  /* CPhysicsActor */
  void Touch(CActor& actor, CStateManager& mgr) override;
  rstl::optional_object<CAABox> GetTouchBounds() const override;

  void RandomizePlaybackRate(CStateManager& mgr);

private:
  enum EAnimationState {
    kAS_Ready,
    kAS_Alert,
    kAS_Impact,
  };

  CHealthInfo x258_initialHealthInfo;
  CHealthInfo x260_healthInfo;
  CDamageVulnerability x268_dVuln;
  EAnimationState x2d0_animState;
  float x2d4_alertRange;
  float x2d8_impactRange;
  s32 x2dc_currentAnim;
  s32 x2e0_alertAnim;
  s32 x2e4_impactAnim;
  bool x2e8_24_dead : 1;
  bool x2e8_25_animating : 1;
};

#endif // _CAMBIENTAI
