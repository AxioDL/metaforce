#ifndef __URDE_CPATTERNED_HPP__
#define __URDE_CPATTERNED_HPP__

#include "CAi.hpp"
#include "Character/CBodyController.hpp"
#include "Character/CSteeringBehaviors.hpp"
#include "Graphics/CVertexMorphEffect.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include "TCastTo.hpp"
#include "CDamageInfo.hpp"

namespace urde
{
class CPatternedInfo;

class CPatternedUnknown2
{
    friend class CPatterned;
    u8 x80_ = 0;
    union
    {
        struct
        {
            bool x81_24_ : 1;
            bool x81_25_ : 1;
            bool x81_26_ : 1;
            bool x81_27_ : 1;
            bool x81_28_ : 1;
            bool x81_29_ : 1;
            bool x81_30_ : 1;
            bool x81_31_ : 1;
            bool x82_24_ : 1;
            bool x82_25_ : 1;
            bool x82_26_ : 1;
        };
        u32 dummy = 0;
    };
};

class CPatterned : public CAi
{
public:
    static const zeus::CColor skDamageColor;
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
    u32 x2d8_ = -1;
    TUniqueId x2dc_ = kInvalidUniqueId;
    zeus::CVector3f x2e0_;
    zeus::CVector3f x2ec_;
    float x2f8_ = 0.f;
    float x2fc_minAttackRange;
    float x300_maxAttackRange;
    float x304_averageAttackTime;
    float x308_attackTimeVariation;
    u32 x30c_ = 0;
    zeus::CVector3f x310_;
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

    u32 x32c_;
    CStateMachineState x330_stateMachineState;
    ECharacter x34c_character;
    zeus::CVector3f x350_;
    zeus::CVector3f x35c_;
    zeus::CVector3f x368_;
    u32 x374_ = 0;
    u32 x378_ = 2;
    u32 x37c_ = 1;
    u32 x380_ = 0;
    u32 x384_ = 0;
    s32 x388_anim;
    /*x38c_*/
    u32 x390_ = 0;
    u32 x394_ = 0;
    u32 x398_ = 0;
    u32 x39c_ = 0;
    zeus::CVector3f x3a0_;
    TUniqueId x3ac_ = kInvalidUniqueId;
    float x3b0_ = 1.f;
    float x3b4_speed;
    float x3b8_turnSpeed;
    float x3bc_detectionRange;
    float x3c0_detectionHeightRange;
    float x3c4_detectionAngle;
    float x3c8_leashRadius;
    float x3cc_playerLeashRadius;
    float x3d0_playerLeashTime;
    float x3d4_ = 0.f;
    float x3d8_;
    float x3dc_;
    float x3e0_;
    float x3e4_ = 0.f;
    float x3e8_ = 0.f;
    float x3ec_ = 0.f;
    float x3f0_ = 0.f;
    float x3f4_ = 0.f;
    u32 x3f8_ = 0;
    EFlavorType x3fc_flavor;

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

    CDamageInfo x404_;
    float x420_ = 0.f;
    float x424_damageWaitTime;
    float x428_ = -1.f;
    zeus::CColor x42c_ = zeus::CColor::skBlack;
    zeus::CColor x430_ = skDamageColor;
    CSteeringBehaviors x45c_;
    std::unique_ptr<CBodyController> x450_bodyController;
    u32 x454_deathSfx;
    u32 x458_iceShatterSfx;

    CPatternedUnknown2 x460_;
    zeus::CVector3f x4e4_;
    float x4f0_ = 0.f;
    float x4f4_;
    float x4f8_;
    float x4fc_;
    float x500_ = 0.f;
    float x504_damageDur = 0.f;
    EColliderType x508_colliderType;
    float x50c_thermalMag;
    std::unique_ptr<CVertexMorphEffect> x510_;
    zeus::CVector3f x514_;
    std::experimental::optional<TLockedToken<CGenDescription>> x520_;
    std::experimental::optional<TLockedToken<CElectricDescription>> x530_;
    zeus::CVector3f x540_;
    std::experimental::optional<TLockedToken<CGenDescription>> x54c_;
    /* x55c_ */
    /* x560_ */
    /* x564_ */
public:
    CPatterned(ECharacter character, TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
               CPatterned::EMovementType movement, EColliderType collider, EBodyType body,
               const CActorParameters& params, int variant);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    void Think(float, CStateManager&);
    void Touch(CActor&, CStateManager&);
    virtual void Death(CStateManager&, const zeus::CVector3f&, EStateMsg) {}
    virtual void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType, bool, float) {}
    zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const
    {
        return GetAimPosition(mgr, 0.f);
    }

    zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const;

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
    zeus::CVector3f GetGunEyePos() const;

    void BuildBodyController(EBodyType);
    const CBodyController* GetBodyController() const { return x450_bodyController.get(); }
    CBodyController* BodyController() { return x450_bodyController.get(); }
    void SetupPlayerCollision(bool);
};
}

#endif // CPATTERNED_HPP
