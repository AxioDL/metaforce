#ifndef _CSCRIPTVISORFLARE
#define _CSCRIPTVISORFLARE

#include "MetroidPrime/CActor.hpp"

#include "MetroidPrime/CVisorFlare.hpp"

class CScriptVisorFlare : public CActor {
  CVisorFlare xe8_flare;
  bool x11c_notInRenderLast;

public:
  CScriptVisorFlare(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool active,
                    const CVector3f& pos, CVisorFlare::EBlendMode blendMode, bool, float, float, float, uint, uint,
                    const rstl::vector<CVisorFlare::CFlareDef>& flares);
  ~CScriptVisorFlare();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
};

#endif // _CSCRIPTVISORFLARE
