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

  float GetMinRange() const { return x104_minRange; }
  float GetMaxRange() const { return x108_maxRange; }
  bool GetViewCheck() const { return x118_24_viewCheck; }
  float GetChanceMinRange() const { return x10c_chanceMinRange; }
  float GetChanceMaxRange() const { return x110_chanceMaxRange; }
  const CColor& GetColor() const { return x114_color; }

  const TToken< CGenDescription >& GetParticleDesc() const { return xe8_particleDesc; }
  const TToken< CElectricDescription >& GetElectricDesc() const { return xf0_electricDesc; }

  private:
    TToken< CGenDescription > xe8_particleDesc;
    TToken< CElectricDescription > xf0_electricDesc;
    ushort xf8_sfx;
    CAssetId xfc_particleId;
    CAssetId x100_electricId;
    float x104_minRange;
    float x108_maxRange;
    float x10c_chanceMinRange;
    float x110_chanceMaxRange;
    CColor x114_color;
    bool x118_24_viewCheck : 1;
  };

#endif // _CSCRIPTVISORGOO
