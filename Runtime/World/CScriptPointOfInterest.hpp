#ifndef __URDE_CSCRIPTPOINTOFINTEREST_HPP__
#define __URDE_CSCRIPTPOINTOFINTEREST_HPP__

#include "CActor.hpp"

namespace urde
{
class CScannableParameters;
class CScriptPointOfInterest : public CActor
{
private:
    float xe8_pointSize;
public:
    CScriptPointOfInterest(TUniqueId, std::string_view, const CEntityInfo, const zeus::CTransform&, bool,
                           const CScannableParameters&, float);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const;
    void Render(const CStateManager &) const;
    void CalculateRenderBounds();
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
};
}
#endif // __URDE_CSCRIPTPOINTOFINTEREST_HPP__
