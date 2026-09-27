#ifndef _CSCRIPTDISTANCEFOG
#define _CSCRIPTDISTANCEFOG

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Math/CVector2f.hpp"

#include "MetroidPrime/CAreaFog.hpp"
#include "MetroidPrime/CGameArea.hpp"

class CScriptDistanceFog : public CEntity {
  ERglFogMode mMode;
  CColor mColor;
  CVector2f mRange;
  float mColorDelta;
  CVector2f mRangeDelta;
  float mThermalTarget;
  float mThermalSpeed;
  float mXrayTarget;
  float mXraySpeed;
  bool mExplicit;
  bool mNonZero;

public:
  CScriptDistanceFog(TUniqueId, const rstl::string&, const CEntityInfo&, ERglFogMode, const CColor&,
                     const CVector2f&, float, CVector2f, const bool, const bool, float, float,
                     float, float);
  ~CScriptDistanceFog();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

#endif // _CSCRIPTDISTANCEFOG
