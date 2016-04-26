#include "CPatterned.hpp"
#include "CPatternedInfo.hpp"

namespace urde
{

CMaterialList gkPatternedGroundMaterialList(EMaterialTypes::ThirtyThree,
                                            EMaterialTypes::Nineteen,
                                            EMaterialTypes::FourtyOne,
                                            EMaterialTypes::ThirtySeven,
                                            EMaterialTypes::Fourty);
CMaterialList gkPatternedFlyerMaterialList(EMaterialTypes::ThirtyThree,
                                           EMaterialTypes::Nineteen,
                                           EMaterialTypes::FourtyOne,
                                           EMaterialTypes::Fourty);

CPatterned::CPatterned(EUnknown, TUniqueId uid, const std::string& name, CPatterned::EFlavorType flavor,
                       const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
                       const CPatternedInfo& pInfo, CPatterned::EMovementType moveType, CPatterned::EColliderType,
                       EBodyType, const CActorParameters& actorParms, u32)
    : CAi(uid, pInfo.xf8_active, name, info, xf, std::move(mData),
          zeus::CAABox(pInfo.xcc_bodyOrigin - zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, 0.f},
                       pInfo.xcc_bodyOrigin + zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, pInfo.xc8_height}),
          pInfo.x0_mass, pInfo.x54_healthInfo, pInfo.x5c_damageVulnerability,
          moveType == EMovementType::Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
          pInfo.xfc_stateMachineId, actorParms, pInfo.xd8_, 0.8f)
{
}

}
