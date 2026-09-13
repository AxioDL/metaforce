#ifndef _CSCRIPTSTEAM
#define _CSCRIPTSTEAM

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

class CScriptSteam : public CScriptTrigger {
public:
  CScriptSteam(TUniqueId, const rstl::string& name, const CEntityInfo& info, const CVector3f& pos,
               const CAABox&, const CDamageInfo& dInfo, const CVector3f& orientedForce,
               unsigned int flags, bool active, CAssetId, float, float, float, float, bool);
  ~CScriptSteam();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  DECLARE_TYPES_MATCH_OR_ACCEPT;

private:
  bool x150_;
  CAssetId x154_texture;
  float x158_strength;
  float x15c_alphaInDur;
  float x160_alphaOutDur;
  float x164_maxDist;
  float x168_ooMaxDist;

  float GetMaxDist() const { return x164_maxDist; }
  float GetStrength() const { return x158_strength; }
  float GetOOMaxDist() const { return x168_ooMaxDist; }
  CAssetId GetTextureId() const { return x154_texture; }
  float GetFadeOutRate() const { return x160_alphaOutDur; }
  float GetFadeInRate() const { return x15c_alphaInDur; }
};

#endif // _CSCRIPTSTEAM
