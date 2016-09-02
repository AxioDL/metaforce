#include "CScriptPlatform.hpp"
#include "Collision/CMaterialList.hpp"

namespace urde
{

static CMaterialList MakePlatformMaterialList()
{
    CMaterialList ret;
    ret.Add(EMaterialTypes::Nineteen);
    ret.Add(EMaterialTypes::FourtyThree);
    ret.Add(EMaterialTypes::FourtyNine);
    ret.Add(EMaterialTypes::FourtyTwo);
    return ret;
}

CScriptPlatform::CScriptPlatform(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParms,
                                 const zeus::CAABox& aabb, float, bool, float, bool active, const CHealthInfo& hInfo,
                                 const CDamageVulnerability& dInfo,
                                 const rstl::optional_object<TLockedToken<CCollidableOBBTreeGroup>>& dcln, bool, u32,
                                 u32)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakePlatformMaterialList(), aabb, SMoverData(15000.f),
                actParms, 0.3f, 0.1f)
{
}
}
