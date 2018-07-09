#include "CParasite.hpp"
#include "Character/CModelData.hpp"
#include "TCastTo.hpp"
#include "World/CActorParameters.hpp"
#include "World/CPatternedInfo.hpp"

namespace urde::MP1 {

CParasite::CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo &info, const zeus::CTransform &xf,
                     CModelData &&mData, const CPatternedInfo &pInfo, u32, float, float, float, float, float, float, float, float,
                     float, float, float, float, float, float, float, float, float, float, bool, u32, const CDamageVulnerability &,
                     const CParasiteInfo &, u16, u16, u16, u32, u32, float, const CActorParameters &aParams)
    : CWallWalker(ECharacter::Parasite, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Ground, EColliderType::One, EBodyType::WallWalker,
                  aParams, -1, 0)
{
}

void CParasite::Accept(IVisitor &visitor) { visitor.Visit(this); }
} // namespace urde::MP1
