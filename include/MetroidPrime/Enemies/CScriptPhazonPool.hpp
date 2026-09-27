#ifndef _CSCRIPTPHAZONPOOL
#define _CSCRIPTPHAZONPOOL

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;

class CScriptPhazonPool : public CScriptTrigger {
public:
  CScriptPhazonPool(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, const CVector3f& scale, bool active, const CAssetId& w1,
                    const CAssetId& w2, const CAssetId& w3, const CAssetId& w4, uint p11,
                    const CDamageInfo& dInfo, const CVector3f& orientedForce,
                    ETriggerFlags triggerFlags, bool p15, float p16, float p17, float p18,
                    float p19);
  ~CScriptPhazonPool() override;

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  rstl::list< rstl::pair< TUniqueId, bool > > mInhabitants;
  rstl::single_ptr< CModelData > mModelData1;
  rstl::single_ptr< CModelData > mModelData2;
  rstl::single_ptr< CElementGen > mElementGen1;
  rstl::single_ptr< CElementGen > mElementGen2;
  CAABox mBounds;
  CVector3f mScale;
  float x19c;
  float x1a0;
  float x1a4;
  float mRotY;
  float mRotZ;
  float x1b0;
  float x1b4;
  float x1b8;
  float x1bc;
  float x1c0;
  float x1c4;
  float x1c8;
  float x1cc;
  float x1d0;
  float x1d4;
  uint x1d8;
  int x1dc;
  bool x1e0_24 : 1;
  bool x1e0_25 : 1;

  void UpdateInhabitants(CStateManager& mgr);
  void UpdateParticleGens(CStateManager& mgr);
  void RemoveInhabitants(CStateManager& mgr);
  void SetEmitParticles(bool val);
};
CHECK_SIZEOF(CScriptPhazonPool, (VERSION >= VERSION_GM8E_02 ? 0x1f8 : 0x1e8))

#endif // _CSCRIPTPHAZONPOOL
