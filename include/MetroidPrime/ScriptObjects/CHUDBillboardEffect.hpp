#ifndef _CHUDBILLBOARDEFFECT
#define _CHUDBILLBOARDEFFECT

#include "MetroidPrime/CEffect.hpp"

class CGenDescription ;
class CElectricDescription;
class CParticleGen;

class CHUDBillboardEffect : public CEffect {
public:
  CHUDBillboardEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                      const rstl::optional_object< TToken< CElectricDescription > >& electric,
                      TUniqueId uid, bool active, const rstl::string& name, float dist,
                      const CVector3f& scale0, const CColor& color, const CVector3f& scale1,
                      const CVector3f& translation);
  ~CHUDBillboardEffect() override;

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool IsElementGen() const { return mIsElementGen; }
  void SetRunIndefinitely(bool b) { mRunIndefinitely = b; }
  CParticleGen* GetParticleGen() const { return mGenerator.get(); }

  static float GetNearClipDistance(const CStateManager& mgr);
  static const CVector3f& GetScaleForPOV(const CStateManager& mgr);

private:
  rstl::single_ptr< CParticleGen > mGenerator;
  CVector3f mTranslation;
  CVector3f mLocalScale;
  bool mRenderAsParticleGen : 1l; // = true;
  bool mEnableRender : 1l; // = false;
  bool mIsElementGen : 1l; // = false;
  bool mRunIndefinitely : 1l; // = false;
  float mTimeoutTimer; // = 0.f;

  static int g_IndirectTexturedBillboardCount;
  static int g_BillboardCount;

  static float CalcGenRate();
};

#endif // _CHUDBILLBOARDEFFECT
