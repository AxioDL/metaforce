#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CPuffer : public CPatterned
{
public:
    DEFINE_PATTERNED(Puffer)

    CPuffer(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
           const CActorParameters&, const CPatternedInfo&, float, CAssetId, const CDamageInfo&, CAssetId,
           float, bool, bool, bool, const CDamageInfo&, s16);
};
}
