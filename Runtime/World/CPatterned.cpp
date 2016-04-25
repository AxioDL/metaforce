#include "CPatterned.hpp"
#include "CPatternedInfo.hpp"

namespace urde
{

CMaterialList gkPatternedGroundMaterialList;
CMaterialList gkPatternedFlyerMaterialList;

CPatterned::CPatterned(EUnknown, TUniqueId uid, const std::string& name, EFlavorType, const CEntityInfo& info,
                       const zeus::CTransform& xf, CModelData&& mData,
                       const CPatternedInfo& pInfo, EMovementType, EColliderType colType,
                       EBodyType, const CActorParameters& actParms)
: CAi(uid, pInfo.xf8_active, name, info, xf, std::move(mData),
      zeus::CAABox(pInfo.xcc_bodyOrigin - zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, 0.f},
                   pInfo.xcc_bodyOrigin + zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, pInfo.xc8_height}),
      pInfo.x0_mass, pInfo.x54_healthInfo, pInfo.x5c_damageVulnerability,
      colType == EColliderType::Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pInfo.xfc_stateMachineId, actParms, pInfo.xd8_, 0.8f)
{
}

}
