#ifndef __URDE_CPATTERNED_HPP__
#define __URDE_CPATTERNED_HPP__

#include "CAi.hpp"

namespace urde
{
class CPatternedInfo;

enum class EBodyType
{
    Two = 2,
    Three
};

class CPatterned : public CAi
{
public:
    enum class EUnknown
    {
        TwentyThree = 23,
        ThirtyNine
    };
    enum class EFlavorType
    {
        Zero = 0
    };
    enum class EMovementType
    {
        Ground = 0,
        Flyer = 1
    };
    enum class EColliderType
    {
        One = 1
    };
private:
public:

    CPatterned(EUnknown unk, TUniqueId uid, const std::string& name, EFlavorType flavor,
               const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
               const CPatternedInfo& pinfo, CPatterned::EMovementType movement, EColliderType collider,
               EBodyType body, const CActorParameters& params, u32 w1);

    virtual void Death(const zeus::CVector3f& , CStateManager& ) {}
    virtual void KnockBack(const zeus::CVector3f &, CStateManager &) {}
};
}

#endif // CPATTERNED_HPP
