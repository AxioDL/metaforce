#ifndef __URDE_CSCRIPTGUNTURRET_HPP__
#define __URDE_CSCRIPTGUNTURRET_HPP__

#include "CPhysicsActor.hpp"
#include "CDamageInfo.hpp"
namespace urde
{

class CScriptGunTurretData
{
    float x0_;
    float x4_;
    float x8_;
    float xc_;
    float x10_;
    float x14_;
    float x18_ = 30.f;
    float x1c_;
    float x20_;
    float x24_;
    float x28_;
    float x2c_;
    float x30_;
    float x34_;
    float x38_;
    bool x3c_;
    u32 x40_;
    CDamageInfo x44_;
    u32 x60_;
    u32 x64_;
    u32 x68_;
    u32 x6c_;
    u32 x70_;
    u32 x74_;
    u32 x78_;
    u16 x7c_;
    u16 x7e_;
    u16 x80_;
    u16 x82_;
    u16 x84_;
    u16 x86_;
    u32 x88_;
    float x8c_;
    u32 x90_;
    u32 x94_;
    u32 x98_;
    float x9c_;
    bool xa0_;
    static constexpr s32 skMinProperties = 43;
public:
    CScriptGunTurretData(CInputStream&, s32);
    const CAssetId& GetPanningEffectRes() const;
    const CAssetId& GetChargingEffectRes() const;
    const CAssetId& GetFrozenEffectRes() const;
    const CAssetId& GetTargettingLightRes() const;
    const CAssetId& GetDeactivateLightRes() const;
    const CAssetId& GetIdleLightRes() const;
    const CDamageInfo& GetProjectileDamage() const;
    const CAssetId& GetProjectileRes() const;
    u16 GetUnFreezeSoundId() const;
    void GetIntoDeactivateDelay() const;
    static s32 GetMinProperties() { return skMinProperties; }
};

class CScriptGunTurret : public CPhysicsActor
{
public:
    enum class ETurretComponent
    {
        Turret,
        Gun
    };
private:
public:
    CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                     const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                     const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                     const CActorParameters& aParms, const CScriptGunTurretData& turretData);

    void Accept(IVisitor&);

};

}

#endif // __URDE_CSCRIPTGUNTURRET_HPP__
