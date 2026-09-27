#ifndef _CSCRIPTDAMAGEABLETRIGGER
#define _CSCRIPTDAMAGEABLETRIGGER

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"

#include "MetroidPrime/CFluidPlaneDoor.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"

class CVisorParameters;

class CScriptDamageableTrigger : public CActor {
public:
  enum ECanOrbit {
    kCO_NoOrbit,
    kCO_Orbit,
  };

  ~CScriptDamageableTrigger() override;
  CScriptDamageableTrigger(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& position, const CVector3f& extent,
                           const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           uint faceFlag, CAssetId patternTex1, CAssetId patternTex2,
                           CAssetId colorTex, ECanOrbit canOrbit, bool active,
                           const CVisorParameters& vParams);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void Render(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CHealthInfo* HealthInfo(CStateManager&) override;
  void Think(float, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

private:
  CFrustumPlanes mFrustum;
  CAABox mBounds;
  CHealthInfo mOrigHInfo;
  CHealthInfo mHInfo;
  CDamageVulnerability mDVuln;
  uint mFaceFlag;
  float mAlpha;
  CTransform4f mFaceDir;
  CTransform4f mFaceDirInv;
  CVector3f mFaceTranslate;
  float mAlphaTimer;
  CFluidPlaneDoor mFluidPlane;
  bool mNotOccluded : 1;
  bool mAlphaOut : 1;
  bool mOutOfFrustum : 1;
  bool mInvulnerable : 1;
  bool mCanOrbit : 1;

  void SetLinkedObjectAlpha(float a, CStateManager& mgr);
  float GetPuddleAlphaScale() const;
  const CFluidPlane& GetFluidPlane() const { return mFluidPlane; }
  const CFrustumPlanes& GetFrustumPlanes() const { return mFrustum; }
};
CHECK_SIZEOF(CScriptDamageableTrigger, (VERSION >= VERSION_GM8E_02 ? 0x318 : 0x308))

#endif // _CSCRIPTDAMAGEABLETRIGGER
