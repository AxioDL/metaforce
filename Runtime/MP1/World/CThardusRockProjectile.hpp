#ifndef __URDE_MP1_CTHARDUSROCKPROJECTILE_HPP__
#define __URDE_MP1_CTHARDUSROCKPROJECTILE_HPP__

#include "World/CPatterned.hpp"

namespace urde
{

class CThardusRockProjectile : public CPatterned
{
public:
    static constexpr ECharacter CharacterType = ECharacter::ThardusRockProjectile;
    CThardusRockProjectile(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& modelData, const CActorParameters& aParms,
                           const CPatternedInfo& patternedInfo, const std::vector<CModelData>& mDataVec, u32);
};

}

#endif // __URDE_MP1_CTHARDUSROCKPROJECTILE_HPP__
