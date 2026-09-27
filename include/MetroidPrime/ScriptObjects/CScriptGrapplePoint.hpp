#ifndef _CSCRIPTGRAPPLEPOINT
#define _CSCRIPTGRAPPLEPOINT

#include "MetroidPrime/CActor.hpp"

#include "MetroidPrime/CGrappleParameters.hpp"

class CScriptGrapplePoint : public CActor {
  CAABox mTouchBounds;
  CGrappleParameters mParameters;

public:
  CScriptGrapplePoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                      const CTransform4f& transform, bool active, const CGrappleParameters& params);
  ~CScriptGrapplePoint();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Render(const CStateManager&) const override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  const CGrappleParameters& GetGrappleParameters() const { return mParameters; }
};

#endif // _CSCRIPTGRAPPLEPOINT
