#ifndef __URDE_CACTOR_HPP__
#define __URDE_CACTOR_HPP__

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
    std::unique_ptr<TToken<CScannableObjectInfo>> x98_scanObjectInfo;
    zeus::CAABox x9c_renderBounds;
    CModelFlags xb4_drawFlags;
    float xbc_time = 0.f;
    float xc0_pitchBend = 0.f;
    TUniqueId xc4_fluidId = kInvalidUniqueId;
    TUniqueId xc6_nextDrawNode = kInvalidUniqueId;
    u32 xc8_drawnToken = -1;
    u32 xcc_addedToken = -1;
    float xd0_;
    float xd4_maxVol = 1.f;
    rstl::reserved_vector<CSfxHandle, 2> xd8_nonLoopingSfxHandles;
    union
    {
        struct
        {
            u8 xe4_24_nextNonLoopingSfxHandle : 3;
            bool xe4_27_ : 1;
            bool xe4_28_ : 1;
            bool xe4_29_actorLightsDirty : 1;
            bool xe4_30_outOfFrustum : 1;
            bool xe4_31_lightsDirty : 1;
            bool xe5_24_ : 1;
            bool xe5_25_ : 1;
            bool xe5_26_muted : 1;
            bool xe5_27_useInSortedLists : 1;
            bool xe5_28_callTouch : 1;
            bool xe5_31_ : 1;
            u8 xe6_24_fluidCounter : 3;
            u8 xe6_27_renderVisorFlags : 3; // 2: thermal cold, 4: thermal hot
            bool xe6_30_enablePitchBend : 1;
            u8 xe6_31_targetableVisorFlags : 4;
            bool xe7_27_ : 1;
            bool xe7_28_worldLightingDirty : 1;
            bool xe7_29_ : 1;
            bool xe7_30_doTargetDistanceTest : 1;
            bool xe7_31_targetable : 1;
        };
        u32 dummy = 0;
    };
    void _CreateShadow();
    void UpdateSfxEmitters();

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

    CActor(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
           const CMaterialList&, const CActorParameters&, TUniqueId);

    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    virtual void SetActive(bool active)
    {
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_actorLightsDirty = true;
        xe7_29_ = true;
        CEntity::SetActive(active);
    }
    virtual void PreRender(CStateManager&, const zeus::CFrustum&) {}
    virtual void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    virtual void Render(const CStateManager&) const {}
    virtual bool CanRenderUnsorted(const CStateManager&) const { return false; }
    virtual void CalculateRenderBounds();
    virtual CHealthInfo* HealthInfo(CStateManager&);
    virtual const CDamageVulnerability* GetDamageVulnerability() const;
    virtual const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                               const CDamageInfo&) const;
    virtual rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    virtual void Touch(CActor&, CStateManager&);
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetHomingPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetScanObjectIndicatorPosition(const CStateManager&) const;
    virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                   const CWeaponMode&, int) const;
    virtual void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
    virtual void OnScanStateChanged(EScanState, CStateManager&);
    virtual zeus::CAABox GetSortingBounds(const CStateManager&) const;
    virtual void DoUserAnimEvent(CStateManager&, CInt32POINode&, EUserEventType, float dt);

    void RemoveEmitter();
    void SetVolume(float vol);
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
    void EnsureRendered(const CStateManager&);
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
    bool GetE7_29() const { return xe7_29_; }
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
};
}

#endif // __URDE_CACTOR_HPP__
