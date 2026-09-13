#include "MetroidPrime/ScriptObjects/CRepulsor.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CRepulsor::CRepulsor(TUniqueId uid, const bool active, const rstl::string& name,
                     const CEntityInfo& info, const CVector3f& pos, float radius)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, xe8_affectRadius(radius) {}

ENTITY_ACCEPT_IMPL(CRepulsor)

void CRepulsor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                CStateManager& stateMgr) {
  CActor::AcceptScriptMsg(msg, objId, stateMgr);
}
