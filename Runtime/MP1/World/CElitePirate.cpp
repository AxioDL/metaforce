#include "CElitePirate.hpp"
#include "World/ScriptLoader.hpp"

namespace urde::MP1
{
CElitePirateData::CElitePirateData(CInputStream& in, u32 propCount)
    : x0_(in.readFloatBig()), x4_(in.readFloatBig()), x8_(in.readFloatBig()), xc_(in.readFloatBig()),
      x10_(in.readFloatBig()), x14_(in.readFloatBig()), x18_(in.readFloatBig()), x1c_(in.readFloatBig()), x20_(in),
      x24_(CSfxManager::TranslateSFXID(in.readUint32Big())), x28_(ScriptLoader::LoadActorParameters(in)),
      x90_(ScriptLoader::LoadAnimationParameters(in)), x9c_(in), xa0_(CSfxManager::TranslateSFXID(in.readUint32Big())),
      xa4_(in), xa8_(in), xc4_(in.readFloatBig()), xc8_(in), xcc_(in), xd0_(in), xd4_(in), xd8_(in.readFloatBig()),
      xdc_(in.readFloatBig()), xe0_(in.readFloatBig()), xe4_(in.readFloatBig()),
      xe8_(zeus::degToRad(in.readFloatBig())), xec_(zeus::degToRad(in.readFloatBig())), xf0_(in.readUint32Big()),
      xf4_(CSfxManager::TranslateSFXID(in.readUint32Big())), xf8_(in), xfc_(in), x118_(in),
      x11c_(CSfxManager::TranslateSFXID(in.readUint32Big())), x11e_(in.readBool()),
      x11f_(propCount < 24 ? true : in.readBool()) {}

CElitePirate::CElitePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CElitePirateData& eliteData)
    : CPatterned(ECharacter::ElitePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Two)
{

}
}
