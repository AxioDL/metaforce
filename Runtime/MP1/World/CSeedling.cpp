#include "MP1/World/CSeedling.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{

CSeedling::CSeedling(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     CAssetId, CAssetId, const CDamageInfo&, const CDamageInfo&, float, float, float, float)
: CWallWalker(ECharacter::Seedling, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo, EMovementType::Ground, EColliderType::Zero, EBodyType::WallWalker, actParms, 0, 4)
{

}

void CSeedling::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
}
