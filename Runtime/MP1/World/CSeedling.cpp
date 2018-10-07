#include "MP1/World/CSeedling.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{

CSeedling::CSeedling(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     CAssetId, CAssetId, const CDamageInfo&, const CDamageInfo&, float f1, float f2, float f3, float f4)
: CWallWalker(ECharacter::Seedling, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
              EMovementType::Ground, EColliderType::Zero, EBodyType::WallWalker, actParms, f1, f2, 0, f3, 4, f4, false)
{

}

void CSeedling::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
}
