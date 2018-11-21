#pragma once

#include "World/CPatterned.hpp"
#include "Character/CBoneTracking.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde::MP1
{
class CEyeball : public CPatterned
{
    float x568_;
    float x56c_;
    CBoneTracking x570_boneTracking;
    zeus::CVector3f x5a8_;
    CProjectileInfo x5b4_projectileInfo;
    CAssetId x5dc_;
    CAssetId x5e0_;
    CAssetId x5e4_;
    CAssetId x5e8_;
    TUniqueId x5ec_projectileId = kInvalidUniqueId;
    u32 x5f0_ = 0;
    u32 x5f4_;
    u32 x5f8_;
    u32 x5fc_;
    u32 x600_;
    s16 x604_;
    CSfxHandle x608_ = 0;
    bool x60c_24_ : 1;
    bool x60c_25_ : 1;
    bool x60c_26_alert : 1;
    bool x60c_27_ : 1;
    bool x60c_28_ : 1;

    void CreateBeam(CStateManager&);

    void TryFlinch(CStateManager&, int);
    void sub802249c8();
public:
    DEFINE_PATTERNED(EyeBall)

    CEyeball(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
             CModelData&&, const CPatternedInfo&, float, float, CAssetId, const CDamageInfo&, CAssetId, CAssetId,
             CAssetId, CAssetId, u32, u32, u32, u32, u32, bool, const CActorParameters&);

    void Accept(IVisitor& visitor);

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void Flinch(CStateManager&, EStateMsg, float);
    void Active(CStateManager&, EStateMsg, float);
    void InActive(CStateManager&, EStateMsg, float);

    void Cover(CStateManager&, EStateMsg, float);

    bool ShouldAttack(CStateManager&, float) { return x60c_26_alert; }
    bool ShouldFire(CStateManager&, float) { return !x60c_27_; }
};
}