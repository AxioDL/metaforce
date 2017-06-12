#include "CPatterned.hpp"
#include "CPatternedInfo.hpp"
#include "TCastTo.hpp"

namespace urde
{

CMaterialList gkPatternedGroundMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                            EMaterialTypes::Orbit, EMaterialTypes::GroundCollider,
                                            EMaterialTypes::Target);
CMaterialList gkPatternedFlyerMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                           EMaterialTypes::Orbit, EMaterialTypes::Target);

CPatterned::CPatterned(ECharacter character, TUniqueId uid, const std::string& name, CPatterned::EFlavorType flavor,
                       const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
                       const CPatternedInfo& pInfo, CPatterned::EMovementType moveType, CPatterned::EColliderType,
                       EBodyType, const CActorParameters& actorParms, bool)
: CAi(uid, pInfo.xf8_active, name, info, xf, std::move(mData),
      zeus::CAABox(pInfo.xcc_bodyOrigin - zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, 0.f},
                   pInfo.xcc_bodyOrigin +
                       zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, pInfo.xc8_height}),
      pInfo.x0_mass, pInfo.x54_healthInfo, pInfo.x5c_damageVulnerability,
      moveType == EMovementType::Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pInfo.xfc_stateMachineId, actorParms, pInfo.xd8_stepUpHeight, 0.8f),
  x34c_character(character)
{
}

}
