set(WEAPON_SOURCES
    WeaponCommon.hpp WeaponCommon.cpp
    CWeaponMgr.hpp CWeaponMgr.cpp
    CGunController.hpp CGunController.cpp
    CGunMotion.hpp CGunMotion.cpp
    CGrappleArm.hpp CGrappleArm.cpp
    CWeaponMode.hpp
    CPlayerGun.hpp CPlayerGun.cpp
    CGunWeapon.hpp CGunWeapon.cpp
    CAuxWeapon.hpp CAuxWeapon.cpp
    CPowerBeam.hpp CPowerBeam.cpp
    CIceBeam.hpp CIceBeam.cpp
    CWaveBeam.hpp CWaveBeam.cpp
    CPlasmaBeam.hpp CPlasmaBeam.cpp
    CPhazonBeam.hpp CPhazonBeam.cpp
    CGSFreeLook.hpp CGSFreeLook.cpp
    CGSComboFire.hpp CGSComboFire.cpp
    CGSFidget.hpp CGSFidget.cpp
    CFidget.hpp CFidget.cpp
    CWeapon.hpp CWeapon.cpp
    CGameProjectile.hpp CGameProjectile.cpp
    CBeamProjectile.hpp CBeamProjectile.cpp
    CElectricBeamProjectile.hpp CElectricBeamProjectile.cpp
    CTargetableProjectile.hpp CTargetableProjectile.cpp
    CBeamInfo.hpp
    CPlasmaProjectile.hpp CPlasmaProjectile.cpp
    CEnergyProjectile.cpp CEnergyProjectile.cpp
    CProjectileWeapon.hpp CProjectileWeapon.cpp
    CBomb.hpp CBomb.cpp
    CPowerBomb.hpp CPowerBomb.cpp
    CFlameInfo.hpp CFlameInfo.cpp
    CFlameThrower.hpp CFlameThrower.cpp
    CWaveBuster.hpp CWaveBuster.cpp
    CNewFlameThrower.hpp CNewFlameThrower.cpp
    CProjectileInfo.hpp CProjectileInfo.cpp
    CBurstFire.hpp CBurstFire.cpp)

runtime_add_list(Weapon WEAPON_SOURCES)
