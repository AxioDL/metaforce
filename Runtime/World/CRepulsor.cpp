#include "Runtime/World/CRepulsor.hpp"

#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CRepulsor::CRepulsor(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                     const zeus::CVector3f& pos, float radius)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_affectRadius(radius) {}

void CRepulsor::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CRepulsor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CActor::AcceptScriptMsg(msg, objId, stateMgr);
}
} // namespace urde
