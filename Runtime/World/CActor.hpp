#pragma once

#include "CEntity.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CSimpleShadow.hpp"
#include "zeus/zeus.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Character/CModelData.hpp"
#include "Character/CActorLights.hpp"
#include "Collision/CCollisionResponseData.hpp"
#include "Audio/CSfxManager.hpp"
#include "CScannableObjectInfo.hpp"
#include "Weapon/WeaponCommon.hpp"

namespace urde
{

class CActorParameters;
class CWeaponMode;
class CHealthInfo;
class CDamageInfo;
class CDamageVulnerability;
class CLightParameters;
class CScriptWater;
class CSimpleShadow;

class CActor : public CEntity
{
    friend class CStateManager;
protected:
    zeus::CTransform x34_transform;
    std::unique_ptr<CModelData> x64_modelData;
    CMaterialList x68_material;
    CMaterialFilter x70_materialFilter;
    s16 x88_sfxId = -1;
    CSfxHandle x8c_loopingSfxHandle;
    std::unique_ptr<CActorLights> x90_actorLights;
    std::unique_ptr<CSimpleShadow> x94_simpleShadow;
    TLockedToken<CScannableObjectInfo> x98_scanObjectInfo;
    zeus::CAABox x9c_renderBounds;
    CModelFlags xb4_drawFlags;
    float xbc_time = 0.f;
    float xc0_pitchBend = 0.f;
    TUniqueId xc4_fluidId = kInvalidUniqueId;
    TUniqueId xc6_nextDrawNode;
    int xc8_drawnToken = -1;
    int xcc_addedToken = -1;
    float xd0_damageMag;
    float xd4_maxVol = 1.f;
    rstl::reserved_vector<CSfxHandle, 2> xd8_nonLoopingSfxHandles;
    union
    {
        struct
        {
            u8 xe4_24_nextNonLoopingSfxHandle : 3;
            bool xe4_27_notInSortedLists : 1;
            bool xe4_28_transformDirty : 1;
            bool xe4_29_actorLightsDirty : 1;
            bool xe4_30_outOfFrustum : 1;
            bool xe4_31_calculateLighting : 1;
            bool xe5_24_shadowEnabled : 1;
            bool xe5_25_shadowDirty : 1;
            bool xe5_26_muted : 1;
            bool xe5_27_useInSortedLists : 1;
            bool xe5_28_callTouch : 1;
            bool xe5_29_globalTimeProvider : 1;
            bool xe5_30_renderUnsorted : 1;
            bool xe5_31_pointGeneratorParticles : 1;
            u8 xe6_24_fluidCounter : 3;
            u8 xe6_27_thermalVisorFlags : 2; // 1: thermal cold, 2: thermal hot
            bool xe6_29_renderParticleDBInside : 1;
            bool xe6_30_enablePitchBend : 1;
            u8 xe6_31_targetableVisorFlags : 4;
            bool xe7_27_enableRender : 1;
            bool xe7_28_worldLightingDirty : 1;
            bool xe7_29_drawEnabled : 1;
            bool xe7_30_doTargetDistanceTest : 1;
            bool xe7_31_targetable : 1;
        };
        u32 dummy = 0;
    };
    void _CreateShadow();
    void UpdateSfxEmitters();
    void DrawTouchBounds() const;
    void RenderInternal(const CStateManager& mgr) const;
    bool IsModelOpaque(const CStateManager& mgr) const;

public:
    enum class EFluidState
    {
        EnteredFluid,
        InFluid,
        LeftFluid
    };

    enum class EScanState
    {
        Start,
        Processing,
        Done,
    };

    CActor(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform&,
           CModelData&& mData, const CMaterialList& list, const CActorParameters& params, TUniqueId otherUid);

    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    virtual void SetActive(bool active)
    {
        xe4_27_notInSortedLists = true;
        xe4_28_transformDirty = true;
        xe4_29_actorLightsDirty = true;
        xe7_29_drawEnabled = active;
        CEntity::SetActive(active);
    }
    virtual void PreRender(CStateManager&, const zeus::CFrustum&);
    virtual void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    virtual void Render(const CStateManager&) const;
    virtual bool CanRenderUnsorted(const CStateManager&) const;
    virtual void CalculateRenderBounds();
    virtual CHealthInfo* HealthInfo(CStateManager&);
    virtual const CDamageVulnerability* GetDamageVulnerability() const;
    virtual const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                               const CDamageInfo&) const;
    virtual std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    virtual void Touch(CActor&, CStateManager&);
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetHomingPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetScanObjectIndicatorPosition(const CStateManager&) const;
    virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                   const CWeaponMode&, EProjectileAttrib) const;
    virtual void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
    virtual void OnScanStateChanged(EScanState, CStateManager&);
    virtual zeus::CAABox GetSortingBounds(const CStateManager&) const;
    virtual void DoUserAnimEvent(CStateManager&, CInt32POINode&, EUserEventType, float dt);

    void RemoveEmitter();
    void SetVolume(float vol);
    void SetMuted(bool);
    const zeus::CTransform& GetTransform() const { return x34_transform; }
    const zeus::CVector3f& GetTranslation() const { return x34_transform.origin; }
    const zeus::CTransform GetScaledLocatorTransform(std::string_view segName) const;
    const zeus::CTransform GetLocatorTransform(std::string_view segName) const;
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, CStateManager&);
    void AddMaterial(const CMaterialList& l);

    void CreateShadow(bool);
    void SetCallTouch(bool callTouch);
    bool GetCallTouch() const;
    void SetUseInSortedLists(bool use);
    bool GetUseInSortedLists() const;
    const CMaterialFilter& GetMaterialFilter() const { return x70_materialFilter; }
    void SetMaterialFilter(const CMaterialFilter& filter) { x70_materialFilter = filter; }
    const CMaterialList& GetMaterialList() const { return x68_material; }
    void SetMaterialList(const CMaterialList& list) { x68_material = list; }
    void SetInFluid(bool in, TUniqueId uid);
    bool HasModelData() const;
    const CSfxHandle& GetSfxHandle() const { return x8c_loopingSfxHandle; }
    void SetSoundEventPitchBend(s32);
    void SetRotation(const zeus::CQuaternion& q);
    void SetTranslation(const zeus::CVector3f& tr);
    void SetTransform(const zeus::CTransform& tr);
    void SetAddedToken(u32 tok);
    float GetPitch() const;
    float GetYaw() const;
    const CModelData* GetModelData() const { return x64_modelData.get(); }
    CModelData* ModelData() { return x64_modelData.get(); }
    void EnsureRendered(const CStateManager&) const;
    void EnsureRendered(const CStateManager&, const zeus::CVector3f&, const zeus::CAABox&) const;
    void ProcessSoundEvent(u32 sfxId, float weight, u32 flags, float falloff, float maxDist,
                           float minVol, float maxVol, const zeus::CVector3f& toListener,
                           const zeus::CVector3f& position, TAreaId aid, CStateManager& mgr,
                           bool translateId);
    SAdvancementDeltas UpdateAnimation(float, CStateManager&, bool);
    void SetActorLights(std::unique_ptr<CActorLights>&& lights);
    const CActorLights* GetActorLights() const { return x90_actorLights.get(); }
    CActorLights* ActorLights() { return x90_actorLights.get(); }
    bool CanDrawStatic() const;
    bool IsDrawEnabled() const { return xe7_29_drawEnabled; }
    void SetWorldLightingDirty(bool b) { xe7_28_worldLightingDirty = b; }
    const CScannableObjectInfo* GetScannableObjectInfo() const;
    const CHealthInfo* GetHealthInfo(const CStateManager& mgr) const
    { return const_cast<CActor*>(this)->HealthInfo(const_cast<CStateManager&>(mgr)); }
    bool GetDoTargetDistanceTest() const { return xe7_30_doTargetDistanceTest; }
    void SetCalculateLighting(bool c);
    float GetAverageAnimVelocity(int anim) const;
    u8 GetTargetableVisorFlags() const { return xe6_31_targetableVisorFlags; }
    bool GetIsTargetable() const { return xe7_31_targetable; }
    void SetDrawFlags(const CModelFlags& flags) { xb4_drawFlags = flags; }
    void SetModelData(std::unique_ptr<CModelData>&& mData);
    u8 GetFluidCounter() const { return xe6_24_fluidCounter; }
    TUniqueId GetFluidId() const { return xc4_fluidId; }
    bool GetPointGeneratorParticles() const { return xe5_31_pointGeneratorParticles; }
    void SetPointGeneratorParticles(bool s) { xe5_31_pointGeneratorParticles = s; }
    CSimpleShadow* Shadow() { return x94_simpleShadow.get(); }
};
}

