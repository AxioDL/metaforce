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

  CHealthInfo mInitialHealthInfo;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDVuln;
  EAnimationState mAnimState;
  float mAlertRange;
  float mImpactRange;
  s32 mCurrentAnim;
  s32 mAlertAnim;
  s32 mImpactAnim;
  bool mDead : 1;
  bool mAnimating : 1;
};

#endif // _CAMBIENTAI
