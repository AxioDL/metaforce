#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CFlickerBat : public CPatterned
{
public:
    DEFINE_PATTERNED(FlickerBat)
    CFlickerBat(TUniqueId, std::string_view name, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
        const CPatternedInfo&, EColliderType, bool, const CActorParameters&, bool);
};
}
