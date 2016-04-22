#include "CScriptGrapplePoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"

namespace urde
{
CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, const std::string &name, const CEntityInfo& info,
                                         const zeus::CTransform &transform, bool active, const CGrappleParameters &params)
    : CActor(uid, active, name, info, transform, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::FourtyOne),
             CActorParameters::None(),
             kInvalidUniqueId),
      xe8_(x34_transform.m_origin - 0.5f,
           x34_transform.m_origin + 0.5f),
      x100_parameters(params)
{
}

}
