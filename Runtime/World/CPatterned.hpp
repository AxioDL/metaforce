#ifndef __URDE_CPATTERNED_HPP__
#define __URDE_CPATTERNED_HPP__

#include "CAi.hpp"
#include "Character/CBodyController.hpp"
#include "TCastTo.hpp"

namespace urde
{
class CPatternedInfo;

class CPatterned : public CAi
{
public:
    enum class ECharacter
    {
        AtomicAlpha = 0,
        AtomicBeta = 1,
        Babygoth = 2,
        Beetle = 3,
        BloodFlower = 4,
        Burrower = 5,
        ChozoGhost = 6,
        Drone = 7,
        ElitePirate = 8,
        EyeBall = 9,
        FireFlea = 10,
        Flaahgra = 11,
        FlaahgraTentacle = 12,
        FlickerBat = 13,
        FlyingPirate = 14,
        IceSheeegoth = 15,
        JellyZap = 16,
        Magdolite = 17,
        Metaree = 18,
        Metroid = 19,
        MetroidBeta = 20,
        MetroidPrimeExo = 21,
        MetroidPrimeEssence = 22,
        NewIntroBoss = 23,
        Parasite = 24,
        PuddleSpore = 27,
        PuddleToad = 28,
        Puffer = 29,
        Ridley = 30,
        Ripper = 31,
        Seedling = 32,
        SpacePirate = 34,
        SpankWeed = 35,
        PhazonHealingNodule = 35,
        Thardus = 36,
        ThardusRockProjectile = 37,
        Tryclops = 38,
        WarWasp = 39,
        EnergyBall = 40
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
    ECharacter x34c_character;
public:
    CPatterned(ECharacter character, TUniqueId uid, const std::string& name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
               CPatterned::EMovementType movement, EColliderType collider, EBodyType body,
               const CActorParameters& params, int variant);

    virtual void Death(CStateManager&, const zeus::CVector3f&, EStateMsg) {}
    virtual void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType, bool, float) {}

    template <class T>
    static T* CastTo(CEntity* ent)
    {
        if (TCastToPtr<CPatterned> patterned = ent)
            if (patterned->x34c_character == T::CharacterType)
                return static_cast<T*>(patterned.GetPtr());
        return nullptr;
    }

    template <class T>
    static const T* CastTo(const CEntity* ent)
    {
        if (TCastToConstPtr<CPatterned> patterned = ent)
            if (patterned->x34c_character == T::CharacterType)
                return static_cast<const T*>(patterned.GetPtr());
        return nullptr;
    }
};
}

#endif // CPATTERNED_HPP
