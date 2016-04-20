#include "CScriptPlatform.hpp"
#include "Collision/CMaterialList.hpp"

namespace urde
{

static CMaterialList MakePlatformMaterialList()
{
    CMaterialList ret;
    ret.x0_ |= 1ull << 19;
    ret.x0_ |= 1ull << 43;
    ret.x0_ |= 1ull << 49;
    ret.x0_ |= 1ull << 42;
    return ret;
}

CScriptPlatform::CScriptPlatform(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, const CModelData& mData,
                                 const CActorParameters& actParms, const zeus::CAABox& aabb,
                                 float, bool, float, bool active, const CHealthInfo& hInfo,
                                 const CDamageVulnerability& dInfo, const TLockedToken<CCollidableOBBTreeGroup>& dcln,
                                 bool, u32, u32)
: CPhysicsActor(uid, active, name, info, xf, mData, MakePlatformMaterialList(),
                aabb, SMoverData(15000.f), actParms, 0.3f, 0.1f)
{
}

}
