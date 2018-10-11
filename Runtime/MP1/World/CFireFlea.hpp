#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CFireFlea : public CPatterned
{
public:
    DEFINE_PATTERNED(FireFlea)

    CFireFlea(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, float);
};
}