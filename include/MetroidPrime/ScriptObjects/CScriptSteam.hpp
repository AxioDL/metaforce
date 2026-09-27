#ifndef _CSCRIPTSTEAM
#define _CSCRIPTSTEAM

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

class CScriptSteam : public CScriptTrigger {
public:
  CScriptSteam(TUniqueId, const rstl::string& name, const CEntityInfo& info, const CVector3f& pos,
               const CAABox&, const CDamageInfo& dInfo, const CVector3f& orientedForce,
               unsigned int flags, bool active, CAssetId, float, float, float, float, bool);
  ~CScriptSteam();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

private:
  bool x150_;
  CAssetId mTexture;
  float mStrength;
  float mAlphaInDur;
  float mAlphaOutDur;
  float mMaxDist;
  float mOoMaxDist;

  float GetMaxDist() const { return mMaxDist; }
  float GetStrength() const { return mStrength; }
  float GetOOMaxDist() const { return mOoMaxDist; }
  CAssetId GetTextureId() const { return mTexture; }
  float GetFadeOutRate() const { return mAlphaOutDur; }
  float GetFadeInRate() const { return mAlphaInDur; }
};

#endif // _CSCRIPTSTEAM
