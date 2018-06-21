#ifndef __URDE_CSCRIPTGRAPPLEPOINT_HPP__
#define __URDE_CSCRIPTGRAPPLEPOINT_HPP__

#include "CActor.hpp"
#include "CGrappleParameters.hpp"

namespace urde
{
class CScriptGrapplePoint : public CActor
{
    zeus::CAABox xe8_touchBounds;
    CGrappleParameters x100_parameters;
public:
    CScriptGrapplePoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                        const zeus::CTransform& transform, bool active, const CGrappleParameters& params);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
    void Render(const CStateManager&) const;
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    const CGrappleParameters& GetGrappleParameters() const { return x100_parameters; }
};
}

#endif // __URDE_CSCRIPTGRAPPLEPOINT_HPP__
