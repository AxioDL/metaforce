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

namespace urde
{
namespace NWeaponTypes
{
enum class EGunAnimType
{
};
}

class CActorLights;
class CGunController;
struct CModelFlags;
class CStateManager;
class CGunWeapon
{
public:
    enum class ESecondaryFxType : u32
    {
    };
protected:
    static const char* skBeamXferNames[5];
    static const char* skAnimDependencyNames[5];
    static const char* skDependencyNames[5];
    static const char* skSuitArmNames[8];
    zeus::CVector3f x4_;
    TToken<CAnimCharacterSet> x104_gunCharacter;
    TToken<CAnimCharacterSet> x13c_armCharacter;
    EWeaponType x1c0_weaponType;
    TUniqueId x1c4_uid;
    EMaterialTypes x1c8_matType;
    CPlayerState::EBeamId x200_beamId;
public:
    CGunWeapon(ResId ancsId, EWeaponType type, TUniqueId uid, EMaterialTypes, const zeus::CVector3f& vec);
    void AsyncLoadSuitArm(CStateManager& mgr);
    void AllocResPools(CPlayerState::EBeamId);

    virtual void Reset(CStateManager&) {}
    virtual void PlayAnim(NWeaponTypes::EGunAnimType, bool) {}
    virtual void PreRenderGunFx(const CStateManager&, const zeus::CTransform&) {}
    virtual void PostRenderGunFx(const CStateManager&, const zeus::CTransform&) {}
    virtual void UpdateGunFx(bool, float, const CStateManager&, const zeus::CTransform&) {}
    virtual void Fire(bool, float, CPlayerState::EChargeState, const zeus::CTransform&, CStateManager&, TUniqueId) {}
    virtual void EnableFx(bool) {}
    virtual void EnableSecondaryFx(ESecondaryFxType) {}
    virtual void Draw(bool, const CStateManager&, const zeus::CTransform&, const CModelFlags&,
                      const CActorLights*) const {}
    virtual void DrawMuzzleFx(const CStateManager&) const {}
    virtual void Update(float, CStateManager&) {}
    virtual void Load(bool) {}
    virtual void Unload(CStateManager&) {}
    virtual bool IsLoaded() const {return false;}
};
}

#endif // __URDE_CGUNWEAPON_HPP__
