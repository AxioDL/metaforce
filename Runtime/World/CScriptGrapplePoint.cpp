#include "CScriptGrapplePoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                         const zeus::CTransform &transform, bool active,
                                         const CGrappleParameters &params)
    : CActor(uid, active, name, info, transform, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Orbit),
             CActorParameters::None(),
             kInvalidUniqueId),
      xe8_(x34_transform.origin - 0.5f,
           x34_transform.origin + 0.5f),
      x100_parameters(params)
{
}

void CScriptGrapplePoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
