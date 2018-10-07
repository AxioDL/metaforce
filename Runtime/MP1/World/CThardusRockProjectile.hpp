#pragma once

#include "World/CPatterned.hpp"

namespace urde
{

class CThardusRockProjectile : public CPatterned
{
public:
    DEFINE_PATTERNED(ThardusRockProjectile)
    CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& modelData, const CActorParameters& aParms,
                           const CPatternedInfo& patternedInfo, const std::vector<CModelData>& mDataVec, u32);
};

}

