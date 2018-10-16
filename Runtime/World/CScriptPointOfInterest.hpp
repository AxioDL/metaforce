#pragma once

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
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
};
}
