#include "CPuffer.hpp"

namespace urde::MP1
{

CPuffer::CPuffer(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
    CModelData&& modelData, const CActorParameters& actorParameters, const CPatternedInfo& patternedInfo,
    float hoverSpeed, CAssetId cloudEffect, const CDamageInfo& cloudDamage, CAssetId cloudSteam, float f2,
                 bool b1, bool b2, bool b3, const CDamageInfo& explosionDamage, s16 sfxId)
: CPatterned(ECharacter::Puffer, uid, name, EFlavorType::Zero, info, xf, std::move(modelData), patternedInfo,
    EMovementType::Flyer, EColliderType::One, EBodyType::RestrictedFlyer, actorParameters, 0)
{

}
}
