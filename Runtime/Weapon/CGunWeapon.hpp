#ifndef __URDE_CGUNWEAPON_HPP__
#define __URDE_CGUNWEAPON_HPP__

#include "RetroTypes.hpp"
#include "CWeaponMgr.hpp"
#include "Collision/CMaterialList.hpp"
#include "zeus/CVector3f.hpp"
#include "CPlayerState.hpp"
#include "Character/CAnimCharacterSet.hpp"
#include "Particle/CElementGen.hpp"
#include "CToken.hpp"
#include "CStateManager.hpp"
#include "CGunController.hpp"
#include "WeaponCommon.hpp"

namespace urde
{

class CActorLights;
struct CModelFlags;
class CWeaponDescription;

using SWeaponInfo = DataSpec::SWeaponInfo;

class CVelocityInfo
{
    friend class CGunWeapon;
    rstl::reserved_vector<zeus::CVector3f, 2> x0_vel;
    rstl::reserved_vector<bool, 2> x1c_;
    rstl::reserved_vector<float, 2> x24_;
public:
    const zeus::CVector3f& GetVelocity(int i) const { return x0_vel[i]; }
};

class CGunWeapon
{
public:
    enum class ESecondaryFxType
    {
        Zero,
        One,
        Two,
        Three
    };
protected:
    static const char* skBeamXferNames[5];
    static const char* skAnimDependencyNames[5];
    static const char* skDependencyNames[5];
    static const char* skSuitArmNames[8];
    zeus::CVector3f x4_scale;
    std::experimental::optional<CModelData> x10_solidModelData;
    std::experimental::optional<CModelData> x60_;
    std::experimental::optional<CModelData> xb0_;
    std::unique_ptr<CGunController> x100_gunController;
    TToken<CAnimCharacterSet> x104_gunCharacter;
    TToken<CAnimCharacterSet> x13c_armCharacter;
    rstl::reserved_vector<TCachedToken<CWeaponDescription>, 2> x144_weapons;
    TToken<CGenDescription> x160_xferEffect;
    rstl::reserved_vector<TCachedToken<CGenDescription>, 2> x16c_muzzleEffects;
    rstl::reserved_vector<TCachedToken<CGenDescription>, 2> x188_secondaryEffects;
    rstl::reserved_vector<std::unique_ptr<CElementGen>, 2> x1a4_muzzleGenerators;
    std::unique_ptr<CElementGen> x1b8_secondaryEffect;
    CRainSplashGenerator* x1bc_rainSplashGenerator = nullptr;
    EWeaponType x1c0_weaponType;
    TUniqueId x1c4_playerId;
    EMaterialTypes x1c8_playerMaterial;
    CVelocityInfo x1d0_velInfo;
    CPlayerState::EBeamId x200_beamId;
    ESecondaryFxType x204_secondaryEffectType = ESecondaryFxType::Zero;
    u32 x208_ = 0;
    CPlayerState::EBeamId x20c_;
    u32 x210_ = 0;
    CAssetId x214_ancsId;
    union
    {
        struct
        {
            bool x218_24 : 1;
            bool x218_25 : 1;
            bool x218_26 : 1;
            bool x218_27 : 1;
            bool x218_28 : 1;
            bool x218_29_leavingBeam : 1;
        };
        u32 _dummy = 0;
    };

public:
    CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId, EMaterialTypes, const zeus::CVector3f& scale);
    void AsyncLoadSuitArm(CStateManager& mgr);
    void AllocResPools(CPlayerState::EBeamId);

    virtual void Reset(CStateManager&) {}
    virtual void PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {}
    virtual void PreRenderGunFx(const CStateManager&, const zeus::CTransform&) {}
    virtual void PostRenderGunFx(const CStateManager&, const zeus::CTransform&) {}
    virtual void UpdateGunFx(bool, float, const CStateManager&, const zeus::CTransform&) {}
    virtual void Fire(bool, float, CPlayerState::EChargeState, const zeus::CTransform&, CStateManager&, TUniqueId) {}
    virtual void EnableFx(bool) {}
    virtual void EnableSecondaryFx(ESecondaryFxType) {}
    void BuildSecondaryEffect(ESecondaryFxType type);
    void ActivateCharge(bool b1, bool b2);
    void Touch(const CStateManager& mgr);
    void TouchHolo(const CStateManager& mgr);
    virtual void Draw(bool, const CStateManager&, const zeus::CTransform&, const CModelFlags&,
                      const CActorLights*) const {}
    virtual void DrawMuzzleFx(const CStateManager&) const {}
    virtual void Update(float, CStateManager&) {}
    virtual void Load(CStateManager& mgr, bool) {}
    virtual void Unload(CStateManager&) {}
    virtual bool IsLoaded() const {return false;}
    void UpdateMuzzleFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos, bool emitting);
    const CVelocityInfo& GetVelocityInfo() const { return x1d0_velInfo; }
    void SetRainSplashGenerator(CRainSplashGenerator* g) { x1bc_rainSplashGenerator = g; }
    CElementGen* GetChargeMuzzleFx() const { return x1a4_muzzleGenerators[1].get(); }
    void ReturnToDefault(CStateManager& mgr) {}
    void UnLoadFidget() {}
    CModelData& GetSolidModelData() { return *x10_solidModelData; }
    const SWeaponInfo& GetWeaponInfo() const;
    zeus::CAABox GetBounds() const;
    void SetLeavingBeam() { x218_29_leavingBeam = true; }
};
}

#endif // __URDE_CGUNWEAPON_HPP__
