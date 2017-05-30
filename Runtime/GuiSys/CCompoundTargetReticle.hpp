#ifndef __CCOMPOUNDTARGETRETICLE_HPP__
#define __CCOMPOUNDTARGETRETICLE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CQuaternion.hpp"
#include "CToken.hpp"

namespace urde
{
class CActor;
class CModel;
class CStateManager;
class CTargetReticleRenderState
{
    TUniqueId x0_target;
    float x4_;
    zeus::CVector3f x8_;
    float x14_;
    float x18_;
    bool x1c_;

public:

    static const CTargetReticleRenderState skZeroRenderState;

    CTargetReticleRenderState(TUniqueId, float, const zeus::CVector3f&, float, float, bool);
    void SetTargetId(TUniqueId);
    void SetFactor(float);
    void SetIsOrbitZoneIdlePosition(bool);
    float GetMinViewportClampScale() const;
    float GetFactor() const;
    float GetRadiusWorld() const;
    const zeus::CVector3f& GetTargetPositionWorld() const;
    bool GetIsOrbitZoneIdlePosition() const;
    void SetTargetPositionWorld(const zeus::CVector3f&);
    void SetRadiusWorld(float);
    TUniqueId GetTargetId() const;
    static void InterpolateWithClamp(const CTargetReticleRenderState&, CTargetReticleRenderState&,
                              const CTargetReticleRenderState&, float);
    float SetMinViewportClampScale(float);
};

class CCompoundTargetReticle
{
public:
    struct SOuterItemInfo
    {
        TToken<CModel> x0_;
        float xc_ = 0.f;
        float x10_ = 0.f;
        float x14_ = 0.f;
        float x18_ = 0.f;
        SOuterItemInfo(const char*);
    };

private:

    static constexpr const char* skCrosshairsReticleAssetName = "CMDL_Crosshairs";
    static constexpr const char* skOrbitZoneReticleAssetName = "CMDL_OrbitZone";
    static constexpr const char* skSeekerAssetName = "CMDL_Seeker";
    static constexpr const char* skLockConfirmAssetName = "CMDL_LockConfirm";
    static constexpr const char* skTargetFlowerAssetName = "CMDL_TargetFlower";
    static constexpr const char* skMissileBracketAssetName = "CMDL_MissileBracket";
    static constexpr const char* skChargeGaugeAssetName = "CMDL_ChargeGauge";
    static constexpr const char* skChargeBeamTickAssetName = "CMDL_ChargeTickFirst";
    static constexpr const char* skOuterBeamIconSquareNameBase = "CMDL_BeamSquare";
    static constexpr const char* skInnerBeamIconName = "CMDL_InnerBeamIcon";
    static constexpr const char* skLockFireAssetName = "CMDL_LockFIre";
    static constexpr const char* skLockDaggerAssetName = "CMDL_LockDagger0";
    static constexpr const char* skGrappleReticleAssetName = "CMDL_Grapple";
    static constexpr const char* skXRayRingModelName = "CMDL_XRayRetRing";
    static constexpr const char* skThermalReticleAssetName = "CMDL_ThermalRet";
    static constexpr const char* skOrbitPointAssetName = "CMDL_OrbitPoint";
    zeus::CQuaternion x0_;
    zeus::CQuaternion x10_;
    u32 x20_ = 5;
    u32 x24_ = 5;
    u32 x28_ = 0;
    float x2c_overshootOffsetHalf;
    float x30_premultOvershootOffset;
    TToken<CModel> x34_crosshairs;
    TToken<CModel> x40_seeker;
    TToken<CModel> x4c_lockConfirm;
    TToken<CModel> x58_targetFlower;
    TToken<CModel> x64_missileBracket;
    TToken<CModel> x70_innerBeamIcon;
    TToken<CModel> x7c_lockFire;
    TToken<CModel> x88_lockDagger;
    TToken<CModel> x94_grapple;
    TToken<CModel> xa0_chargeTickFirst;
    TToken<CModel> xac_xrayRetRing;
    TToken<CModel> xb8_thermalReticle;
    SOuterItemInfo xc4_chargeGauge;
    std::vector<SOuterItemInfo> xe0_outerBeamIconSquares;
    TUniqueId xf0_;
    TUniqueId xf2_;
    zeus::CVector3f xf4_;
    zeus::CVector3f x100_;
    CTargetReticleRenderState x10c_ = CTargetReticleRenderState::skZeroRenderState;
    CTargetReticleRenderState x12c_ = CTargetReticleRenderState::skZeroRenderState;
    CTargetReticleRenderState x14c_ = CTargetReticleRenderState::skZeroRenderState;
    float x16c_ = 0.f;
    float x170_ = 0.f;
    CTargetReticleRenderState x174_ = CTargetReticleRenderState::skZeroRenderState;
    CTargetReticleRenderState x194_ = CTargetReticleRenderState::skZeroRenderState;
    CTargetReticleRenderState x1b4_ = CTargetReticleRenderState::skZeroRenderState;
    float x1d4_ = 0.f;
    float x1d8_ = 0.f;
    TUniqueId x1dc_ = kInvalidUniqueId;
    TUniqueId x1de_ = kInvalidUniqueId;
    float x1e0_ = 0.f;
    float x1e4_ = 0.f;
    float x1e8_ = 0.f;
    float x1ec_ = 0.f;
    float x1f0_ = 0.f;
    u8 x1f4_ = 0;
    float x1f8_ = 0.f;
    float x1fc_ = 0.f;
    u32 x200_ = 0;
    float x204_ = 0.f;
    float x208_;
    float x20c_ = 0.f;
    float x210_ = 0.f;
    float x214_ = 0.f;
    u8 x218_ = 0;
    u8 x219_ = 0;
    u8 x21a_ = 0;
    u8 x21b_ = 0;
    u32 x21c_;
    u32 x220_;
    u32 x228_;
public:
    CCompoundTargetReticle(const CStateManager&);

    void SetLeadingOrientation(const zeus::CQuaternion&);
    bool CheckLoadComplete() { return true; }
    void GetDesiredReticleState(const CStateManager&) const;
    void Update(float, const CStateManager&);
    void UpdateCurrLockOnGroup(float, const CStateManager&);
    void UpdateUpdateNextLockOnGroup(float, const CStateManager&);
    void UpdateOrbitZoneGroup(float, const CStateManager&);
    void Draw(const CStateManager&) const;
    void DrawCurrLockOnGroup(const CStateManager&) const;
    void DrawNextLockOnGroup(const CStateManager&) const;
    void DrawOrbitZoneGroup(const CStateManager&) const;
    void UpdateTargetParameters(CTargetReticleRenderState&, const CStateManager&);
    float CalculateRadiusWorld(const CActor&, const CStateManager&) const;
    zeus::CVector3f CalculatePositionWorld(const CActor&, const CStateManager&) const;
    zeus::CVector3f CalculateOrbitZoneReticlePosition(const CStateManager&) const;
    bool IsGrappleTarget(TUniqueId, const CStateManager&) const;
    static float CalculateClampedScale(const zeus::CVector3f&, float, float, float, const CStateManager&);
    void Touch() const;
};
}

#endif // __CCOMPOUNDTARGETRETICLE_HPP__
