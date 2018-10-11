#include "MP1/World/CRidley.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{

CRidleyData::CRidleyData(CInputStream& in, u32 propCount)
    : x0_(in), x4_(in), x8_(in), xc_(in), x10_(in), x14_(in), x18_(in), x1c_(in), x20_(in), x24_(in), x28_(in),
      x2c_(in), x30_(in), x34_(in.readFloatBig()), x38_(in.readFloatBig()), x3c_(in.readFloatBig()),
      x40_(in.readFloatBig()), x44_(in), x48_(in), x64_(in), xa8_(CSfxManager::TranslateSFXID(in.readUint32Big())),
      xac_(in), xb0_(in), xcc_(in), x1a0_(in), x1a4_(in), x1c0_(in),
      x294_(CSfxManager::TranslateSFXID(in.readUint32Big())), x298_(in), x2b4_(in), x388_(in.readFloatBig()),
      x38c_(in.readFloatBig()), x390_(in), x3ac_(in.readFloatBig()), x3b0_(in), x3cc_(in.readFloatBig()), x3d0_(in),
      x3f4_(in.readFloatBig()), x3f8_(CSfxManager::TranslateSFXID(in.readUint32Big())),
      x3fc_(propCount > 47 ? CDamageInfo(in) : x48_)
{
}

CRidley::CRidley(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                 const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                 const CActorParameters& actParms, CInputStream& in, u32 propCount)
    : CPatterned(ECharacter::Ridley, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::Zero, EBodyType::Flyer, actParms, 2), x568_(in, propCount)
{
}
}
}
