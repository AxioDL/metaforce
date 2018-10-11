#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CAtomicAlpha : public CPatterned
{
public:
    DEFINE_PATTERNED(AtomicAlpha)
    CAtomicAlpha(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CActorParameters&, const CPatternedInfo&, CAssetId, const CDamageInfo&, float, float, float,
                 CAssetId, bool, bool);
};
}

