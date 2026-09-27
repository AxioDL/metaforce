#ifndef _CSCRIPTVISORGOO
#define _CSCRIPTVISORGOO

#include "MetroidPrime/CActor.hpp"

class CGenDescription;
class CElectricDescription;

class CScriptVisorGoo : public CActor {
public:
  CScriptVisorGoo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, CAssetId particle, CAssetId electric, float minRange,
                  float maxRange, float chanceMinRange, float chanceMaxRange, const CColor& color,
                  int sfx, bool noViewCheck, bool active);
  ~CScriptVisorGoo();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;

  float GetMinRange() const { return mMinRange; }
  float GetMaxRange() const { return mMaxRange; }
  bool GetViewCheck() const { return mViewCheck; }
  float GetChanceMinRange() const { return mChanceMinRange; }
  float GetChanceMaxRange() const { return mChanceMaxRange; }
  const CColor& GetColor() const { return mColor; }

  const TToken< CGenDescription >& GetParticleDesc() const { return mParticleDesc; }
  const TToken< CElectricDescription >& GetElectricDesc() const { return mElectricDesc; }

  private:
    TToken< CGenDescription > mParticleDesc;
    TToken< CElectricDescription > mElectricDesc;
    ushort mSfx;
    CAssetId mParticleId;
    CAssetId mElectricId;
    float mMinRange;
    float mMaxRange;
    float mChanceMinRange;
    float mChanceMaxRange;
    CColor mColor;
    bool mViewCheck : 1;
  };

#endif // _CSCRIPTVISORGOO
