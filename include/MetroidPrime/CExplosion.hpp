#ifndef _CEXPLOSION
#define _CEXPLOSION

#include "types.h"

#include "MetroidPrime/CEffect.hpp"

#include "rstl/single_ptr.hpp"

class CParticleGen;
class CGenDescription;
class CElectricDescription;

class CExplosion : public CEffect {
public:
  CExplosion(const TLockedToken< CGenDescription >& particle, TUniqueId uid, bool active,
             const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf, uint flags,
             const CVector3f& scale, const CColor& color);

  CExplosion(const TLockedToken< CElectricDescription >& electric, TUniqueId uid, bool active,
             const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf, uint flags,
             const CVector3f& scale, const CColor& color);

  // CEntity
  ~CExplosion() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  void CalculateRenderBounds() override;

  TUniqueId GetExplosionLightId() const { return mExplosionLight; }
  CAssetId GetSourceId() const { return mSourceId; }

public:
  rstl::single_ptr< CParticleGen > mParticleGen;
  TUniqueId mExplosionLight;
  CAssetId mSourceId;
  bool mRenderThermalHot : 1;
  bool mHasRenderBounds : 1;
  bool mRenderXray : 1;
  float mTime;
};
CHECK_SIZEOF(CExplosion, (VERSION >= VERSION_GM8E_02 ? 0x110 : 0x100))

#endif // _CEXPLOSION
