#ifndef __URDE_CPATTERNED_HPP__
#define __URDE_CPATTERNED_HPP__

#include "CAi.hpp"
#include "Character/CBodyController.hpp"

namespace urde
{
class CPatternedInfo;

class CPatterned : public CAi
{
public:
    enum class EUnknown
    {
        Zero = 0,
        Three = 3,
        TwentyThree = 23,
        ThirtyFour = 34,
        ThirtyNine
    };
    enum class EFlavorType
    {
        Zero = 0,
        One = 1
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
    CPatterned(EUnknown unk, TUniqueId uid, const std::string& name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
               CPatterned::EMovementType movement, EColliderType collider, EBodyType body,
               const CActorParameters& params, bool b1);

    virtual void Death(const zeus::CVector3f&, CStateManager&) {}
    virtual void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType, bool, float) {}
};
}

#endif // CPATTERNED_HPP
