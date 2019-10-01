#include "CIceImpact.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CIceImpact::CIceImpact(const TLockedToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid, bool active,
                       std::string_view name, const zeus::CTransform& xf, u32 flags, const zeus::CVector3f& scale,
                       const zeus::CColor& color)
: CEffect(uid, CEntityInfo(aid, CEntity::NullConnectionList), active, name, xf) {}

void CIceImpact::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
