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

  CVector3f xe8_wallIntersection;
  TUniqueId xf4_scannerLight;
  TLockedToken< CGenDescription > xf8_beamDesc;
  rstl::single_ptr< CElementGen > x104_beamParticle;
};
CHECK_SIZEOF(CDroneLaser, (VERSION >= VERSION_GM8P_00 ? 0x118 : 0x108))

#endif // _CDRONELASER
