#include "CParasite.hpp"
#include "World/CActorParameters.hpp"
#include "Character/CModelData.hpp"
#include "World/CPatternedInfo.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{
CParasite::CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo)
    : CWallWalker(ECharacter::Parasite, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Ground, EColliderType::One, EBodyType::WallWalker, CActorParameters::None(), -1, false)
{

}

void CParasite::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}
}
