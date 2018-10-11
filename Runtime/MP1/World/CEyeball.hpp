#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CEyeball : public CPatterned
{
public:
    DEFINE_PATTERNED(EyeBall)

    CEyeball(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
             CModelData&&, const CPatternedInfo&, float, float, CAssetId, const CDamageInfo&, CAssetId, CAssetId,
             CAssetId, CAssetId, u32, u32, u32, u32, u32, const CActorParameters&, bool);

};
}