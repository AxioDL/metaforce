#ifndef _CDRONELASER
#define _CDRONELASER

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActor.hpp"

class CDroneLaser : public CActor {
public:
  ~CDroneLaser() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;

  CDroneLaser(TUniqueId uid, TAreaId areaId, const CTransform4f& xf, CAssetId particle);
  void SetWallIntersection(CStateManager& mgr, const CVector3f& pos, const CVector3f& direction);

private:
  void SetScannerLightActive(CStateManager& mgr, bool active);
  void RenderBeam(uint subdivisions, float radius, const CColor& color, bool additive) const;

  CVector3f mWallIntersection;
  TUniqueId mScannerLight;
  TLockedToken< CGenDescription > mBeamDesc;
  rstl::single_ptr< CElementGen > mBeamParticle;
};
CHECK_SIZEOF(CDroneLaser, (VERSION >= VERSION_GM8E_02 ? 0x118 : 0x108))

#endif // _CDRONELASER
