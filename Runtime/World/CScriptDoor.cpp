#include "CScriptDoor.hpp"
#include "Collision/CMaterialList.hpp"

namespace urde
{

static CMaterialList MakeDoorMaterialList(bool material)
{
    CMaterialList ret;
    if (material)
    {
        ret.x0_ |= 1ull << 19;
        ret.x0_ |= 1ull << 43;
        ret.x0_ |= 1ull << 41;
    }
    else
    {
        ret.x0_ |= 1ull << 43;
        ret.x0_ |= 1ull << 42;
        ret.x0_ |= 1ull << 19;
        ret.x0_ |= 1ull << 41;
    }
    return ret;
}

CScriptDoor::CScriptDoor(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, const CModelData& mData, const CActorParameters& actParms,
                         const zeus::CVector3f&, const zeus::CAABox& aabb, bool active,
                         bool material, bool, float, bool ballDoor)
: CPhysicsActor(uid, active, name, info, xf, mData, MakeDoorMaterialList(material),
                aabb, SMoverData(1.f), actParms, 0.3f, 0.1f)
{

}

}
