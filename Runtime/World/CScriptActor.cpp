#include "CScriptActor.hpp"

namespace urde
{

CScriptActor::CScriptActor(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CTransform& xf, const CModelData& mData, const zeus::CAABox& aabb, float a, float,
                           const CMaterialList& matList, const CHealthInfo&, const CDamageVulnerability&,
                           const CActorParameters& actParms, bool, bool active, u32, float, bool, bool, bool, bool)
: CPhysicsActor(uid, active, name, info, xf, mData, matList, aabb, SMoverData(a), actParms, 0.3f, 0.1f)
{
}

}
