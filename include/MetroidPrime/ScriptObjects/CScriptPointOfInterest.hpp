#ifndef _CSCRIPTPOINTOFINTEREST
#define _CSCRIPTPOINTOFINTEREST

#include "MetroidPrime/CActor.hpp"

class CScannableParameters;

class CScriptPointOfInterest : public CActor {
private:
  float mPointSize;

public:
  CScriptPointOfInterest(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&, bool,
                         const CScannableParameters&, float);
  ~CScriptPointOfInterest();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object<CAABox> GetTouchBounds() const override;
};

#endif // _CSCRIPTPOINTOFINTEREST
