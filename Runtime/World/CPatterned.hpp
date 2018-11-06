#pragma once

#include "CAi.hpp"
#include "Character/CBodyController.hpp"
#include "Character/CSteeringBehaviors.hpp"
#include "Graphics/CVertexMorphEffect.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include "TCastTo.hpp"
#include "CDamageInfo.hpp"
#include "CKnockBackController.hpp"

#ifndef DEFINE_PATTERNED
#define DEFINE_PATTERNED(type) static constexpr ECharacter CharacterType = ECharacter::type;
#endif

namespace urde
{
class CPatternedInfo;
class CProjectileInfo;

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
    zeus::CVector3f x2e0_destPos;
    zeus::CVector3f x2ec_;
    float x2f8_ = 0.f;
    float x2fc_minAttackRange;
    float x300_maxAttackRange;
    float x304_averageAttackTime;
    float x308_attackTimeVariation;
    u32 x30c_ = 0;
    zeus::CVector3f x310_;
    zeus::CVector3f x31c_;
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
    float x3d4_curPlayerLeashTime = 0.f;
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
            bool x400_25_alive : 1; // t
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
            bool x402_31_thawed : 1;
            bool x403_24_keepThermalVisorState : 1;
            bool x403_25_ : 1; // t
            bool x403_26_ : 1; // t
        };
        u32 _dummy2 = 0;
    };

    CDamageInfo x404_contactDamage;
    float x420_curDamageTime = 0.f;
    float x424_damageWaitTime;
    float x428_ = -1.f;
    zeus::CColor x42c_ = zeus::CColor::skBlack;
    zeus::CColor x430_ = skDamageColor;
    zeus::CVector3f x434_posDelta;
    zeus::CQuaternion x440_rotDelta;
    CSteeringBehaviors x45c_;
    std::unique_ptr<CBodyController> x450_bodyController;
    u16 x454_deathSfx;
    u16 x458_iceShatterSfx;

    CKnockBackController x460_knockBackController;
    zeus::CVector3f x4e4_;
    float x4f0_ = 0.f;
    float x4f4_;
    float x4f8_;
    float x4fc_;
    float x500_ = 0.f;
    float x504_damageDur = 0.f;
    EColliderType x508_colliderType;
    float x50c_thermalMag;
    std::shared_ptr<CVertexMorphEffect> x510_vertexMorph;
    zeus::CVector3f x514_deathExplosionOffset;
    std::experimental::optional<TLockedToken<CGenDescription>> x520_deathExplosionParticle;
    std::experimental::optional<TLockedToken<CElectricDescription>> x530_deathExplosionElectric;
    zeus::CVector3f x540_iceDeathExplosionOffset;
    std::experimental::optional<TLockedToken<CGenDescription>> x54c_iceDeathExplosionParticle;
    zeus::CVector3f x55c_;
    void UpdateFrozenState(bool thawed);
    void GenerateIceDeathExplosion(CStateManager& mgr);
    void GenerateDeathExplosion(CStateManager& mgr);
public:
    CPatterned(ECharacter character, TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
               CPatterned::EMovementType movement, EColliderType collider, EBodyType body,
               const CActorParameters& params, EKnockBackVariant kbVariant);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreThink(float, CStateManager& mgr) { CEntity::Think(x500_, mgr); }
    void Think(float, CStateManager&);
    void PreRender(CStateManager&, const zeus::CFrustum&);

    void Touch(CActor&, CStateManager&);
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    bool CanRenderUnsorted(const CStateManager& mgr) const;
    zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const
    {
        return GetAimPosition(mgr, 0.f);
    }

    zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const;

    void DeathDelete(CStateManager& mgr);
    void Death(CStateManager& mgr, const zeus::CVector3f&, EScriptObjectState) {}
    void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info,
                   EKnockBackType type, bool inDeferred, float magnitude);
    void TakeDamage(const zeus::CVector3f&, float) { x428_ = 0.33f;}
    bool FixedRandom(CStateManager&, float) { return x330_stateMachineState.GetRandom() < x330_stateMachineState.x14_; }
    bool Random(CStateManager&, float dt) { return x330_stateMachineState.GetRandom() < dt; }
    //bool FixedDelay(CStateManager&, float dt) { return x330_stateMachineState.GetDelay() == dt; }

    bool Default() { return true; }
    virtual bool KnockbackWhenFrozen() const { return true;}
    virtual void DoDeath(CStateManager&);
    virtual void DoIceDeath(CStateManager&);
    virtual void Burn(float, float) {}
    virtual void Shock(float, float) {}
    virtual void Freeze(CStateManager& mgr, const zeus::CVector3f& pos,
                        const zeus::CUnitVector3f& dir, float magnitude) {}
    virtual void ThinkAboutMove(float);
    virtual void GetSearchPath() {}
    virtual CDamageInfo GetContactDamage() const { return x404_contactDamage; }
    virtual u8 GetModelAlphau8(const CStateManager&) const { return u8(x42c_.a * 255);}
    virtual bool IsOnGround() const { return x328_27_onGround; }
    virtual float GetGravityConstant() const { return 24.525002f; }
    virtual CProjectileInfo* GetProjectileInfo() { return nullptr; }
    virtual void PhazeOut(CStateManager&) {}
    virtual const std::experimental::optional<TLockedToken<CGenDescription>>&
    GetDeathExplosionParticle() const { return x520_deathExplosionParticle; }
    float GetDamageDuration() const { return x504_damageDur; }
    zeus::CVector3f GetGunEyePos() const;
    bool IsAlive() const { return x400_25_alive; }

    void BuildBodyController(EBodyType);
    const CBodyController* GetBodyController() const { return x450_bodyController.get(); }
    CBodyController* BodyController() { return x450_bodyController.get(); }
    const CKnockBackController& GetKnockBackController() const { return x460_knockBackController; }
    void SetupPlayerCollision(bool);


    void SetDestPos(const zeus::CVector3f& pos) { x2e0_destPos = pos; }
    void sub8007a68c(float, CStateManager&) {}
    float sub80078a88();
    void sub8007a5b8(float) {}

    bool GetX328_26() const { return x328_26_; }
    bool GetX402_28() const { return x402_28_; }

    //region Casting Functions

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

    //endregion
};
}

