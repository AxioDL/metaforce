#include "CScriptDebris.hpp"

namespace urde
{

CScriptDebris::CScriptDebris(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& aParams, ResId, const zeus::CVector3f&, float,
                             const zeus::CVector3f&, const zeus::CColor&, float f1, float f2, float f3,
                             CScriptDebris::EScaleType, bool, bool, bool active)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData),
                    CMaterialList(EMaterialTypes::Nineteen, EMaterialTypes::FourtyFour),
                    mData.GetBounds(xf.getRotation()), SMoverData(f2), aParams, 0.3, 0.1)
{

}

#if 0
CScriptDebris::CScriptDebris(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                             const CActorParameters&, float, float, float, float, float, float, float, float, float,
                             const zeus::CColor&, const zeus::CColor&, float, const zeus::CVector3f&, const zeus::CVector3f&,
                             float, float, const zeus::CVector3f&, u32, const zeus::CVector3f&, bool, bool,
                             CScriptDebris::EOrientationType, u32, const zeus::CVector3f&, bool, bool,
                             CScriptDebris::EOrientationType, u32, const zeus::CVector3f&, CScriptDebris::EOrientationType,
                             bool, bool, bool)
{
}
#endif

}
