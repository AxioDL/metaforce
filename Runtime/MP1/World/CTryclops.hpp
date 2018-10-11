#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CTryclops : public CPatterned
{
public:
    DEFINE_PATTERNED(Tryclops)
    CTryclops(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
        const CPatternedInfo&, const CActorParameters&, float, float, float, float);
};
}
