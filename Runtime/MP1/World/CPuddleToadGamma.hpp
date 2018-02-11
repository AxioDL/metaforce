#ifndef __URDE_MP1_CPUDDLETOADGAMMA_HPP__
#define __URDE_MP1_CPUDDLETOADGAMMA_HPP__

#include "World/CPatterned.hpp"

namespace urde::MP1
{

class CPuddleToadGamma : public CPatterned
{
public:
    static constexpr ECharacter CharacterType = ECharacter::PuddleToad;
    CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                     const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& aParms, float f1, float f2, float f3, const zeus::CVector3f& v1,
                     float f4, float f5, float f6, const CDamageInfo& dInfo1, const CDamageInfo& dInfo2);
};

}

#endif // __URDE_MP1_CPUDDLETOADGAMMA_HPP__
