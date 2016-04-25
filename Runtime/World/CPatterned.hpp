#ifndef __URDE_CPATTERNED_HPP__
#define __URDE_CPATTERNED_HPP__

#include "CAi.hpp"

namespace urde
{
class CPatternedInfo;

enum class EBodyType
{
    Two = 2
};

class CPatterned : public CAi
{
public:
    enum class EUnknown
    {
        TwentyThree = 23
    };
    enum class EFlavorType
    {
        Zero = 0
    };
    enum class EMovementType
    {
        One = 1
    };
    enum class EColliderType
    {
        Ground = 0,
        Flyer = 1
    };
private:
public:
    CPatterned(EUnknown, TUniqueId, const std::string& name, EFlavorType, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData,
               const CPatternedInfo& pInfo, EMovementType, EColliderType,
               EBodyType, const CActorParameters& actParms);
};

}

#endif // __URDE_CPATTERNED_HPP__
