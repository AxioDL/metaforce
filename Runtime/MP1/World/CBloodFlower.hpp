#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CBloodFlower : public CPatterned
{
public:
    DEFINE_PATTERNED(BloodFlower)

    CBloodFlower(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CPatternedInfo&, CAssetId, CAssetId, const CActorParameters&, CAssetId, const CDamageInfo&,
                 const CDamageInfo&, const CDamageInfo&, CAssetId, CAssetId, CAssetId, float, CAssetId, u32);
};
}
