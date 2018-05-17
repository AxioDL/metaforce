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
        Zero = 0,
        One = 1
    };

protected:
    union
    {
        struct
        {
            bool x328_24_ : 1;
            bool x328_25_ : 1;
            bool x328_26_ : 1;
            bool x328_27_onGround : 1;
        };
        u32 _dummy = 0;
    };
    ECharacter x34c_character;

    float x338_;

    union
    {
        struct
        {
            bool x400_24_ : 1;
            bool x400_25_ : 1; // t
            bool x400_26_ : 1;
            bool x400_27_ : 1;
            bool x400_28_ : 1;
            bool x400_29_ : 1;
            bool x400_30_ : 1;
            bool x400_31_ : 1; // r25 == 1
            bool x401_24_ : 1;
            bool x401_25_ : 1;
            bool x401_26_ : 1;
            bool x401_27_ : 1;
            bool x401_28_ : 1;
            bool x401_29_ : 1;
            bool x401_30_ : 1;
            bool x401_31_ : 1;
            bool x402_24_ : 1;
            bool x402_25_ : 1;
            bool x402_26_ : 1;
            bool x402_27_ : 1;
            bool x402_28_ : 1;
            bool x402_29_ : 1; // t
            bool x402_30_ : 1;
            bool x402_31_ : 1;
            bool x403_24_ : 1;
            bool x403_25_ : 1; // t
            bool x403_26_ : 1; // t
        };
        u32 _dummy2 = 0;
    };

    std::unique_ptr<CBodyController> x450_bodyController;

    union
    {
        struct
        {
            bool x4e1_24_ : 1;
            bool x4e1_25_ : 1;
            bool x4e1_26_ : 1;
        };
        u32 _dummy3 = 0;
    };

    float x500_ = 0.f;
    float x504_damageDur = 0.f;
public:
    CPatterned(ECharacter character, TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
               CPatterned::EMovementType movement, EColliderType collider, EBodyType body,
               const CActorParameters& params, int variant);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    virtual void Death(CStateManager&, const zeus::CVector3f&, EStateMsg) {}
    virtual void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType, bool, float) {}

    template <class T>
    static T* CastTo(CEntity* ent)
    {
        if (TCastToPtr<CPatterned> patterned = ent)
            return CastTo<T>(patterned.GetPtr());
        return nullptr;
    }

    template <class T>
    static const T* CastTo(const CEntity* ent)
    {
        if (TCastToConstPtr<CPatterned> patterned = ent)
            return CastTo<T>(patterned.GetPtr());
        return nullptr;
    }

    template <class T>
    static T* CastTo(CPatterned* patterned)
    {
        if (patterned->x34c_character == T::CharacterType)
            return static_cast<T*>(patterned);
        return nullptr;
    }

    template <class T>
    static const T* CastTo(const CPatterned* patterned)
    {
        if (patterned->x34c_character == T::CharacterType)
            return static_cast<const T*>(patterned);
        return nullptr;
    }

    bool GetX328_26() const { return x328_26_; }
    bool GetX402_28() const { return x402_28_; }

    virtual bool IsOnGround() const { return x328_27_onGround; }
    virtual float GetGravityConstant() const { return 24.525002f; }
    float GetDamageDuration() const { return x504_damageDur; }

    const CBodyController* GetBodyController() const { return x450_bodyController.get(); }
    CBodyController* BodyController() { return x450_bodyController.get(); }
};
}

#endif // CPATTERNED_HPP
