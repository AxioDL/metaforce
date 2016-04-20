#include "CScriptGrapplePoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"

namespace urde
{
CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, const std::string &name, const CEntityInfo& info,
                                         const zeus::CTransform &transform, bool active, const CGrappleParameters &params)
    : CActor(uid, active, name, info, transform, CModelData::CModelDataNull(), CMaterialList(41), CActorParameters::None(),
             kInvalidUniqueId),
      xe8_({x40_unknown - 0.5f, x50_unknown - 0.5f, x60_unknown - 0.5f},
           {x40_unknown + 0.5f, x50_unknown + 0.5f, x60_unknown + 0.5f}),
      x100_parameters(params)
{
}

}
