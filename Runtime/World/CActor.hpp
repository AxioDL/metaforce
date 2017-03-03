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

namespace urde
{

class CActorParameters;
class CWeaponMode;
class CHealthInfo;
class CDamageInfo;
class CDamageVulnerability;
class CLightParameters;
class CScannableObjectInfo;
class CScriptWater;
class CSimpleShadow;

class CActor : public CEntity
{
protected:
    zeus::CTransform x34_transform;
    std::unique_ptr<CModelData> x64_modelData;
    CMaterialList x68_material;
    CMaterialFilter x70_materialFilter;
    s16 x88_sfxId = -1;
    std::unique_ptr<CSfxHandle> x8c_sfxHandle;
    std::unique_ptr<CActorLights> x90_actorLights;
    std::unique_ptr<CSimpleShadow> x94_simpleShadow;
    std::unique_ptr<TToken<CScannableObjectInfo>> x98_scanObjectInfo;
    zeus::CAABox x9c_aabox;
    u8 xb4_ = 0;
    u8 xb5_ = 0;
    u16 xb6_ = 3;
    zeus::CColor xb8_ = zeus::CColor::skWhite;
    float xbc_time = 0.f;
    s32 xc0_ = 0;
    TUniqueId xc4_fluidId = kInvalidUniqueId;
    TUniqueId xc6_ = kInvalidUniqueId;
    s32 xc8_ = -1;
    s32 xcc_addedToken = -1;
    float xd0_;
    u8 xd4_ = 0x7F;
    u32 xd8_ = 2;
    union
    {
        struct
        {
            bool xe4_27_ : 1;
            bool xe4_28_ : 1;
            bool xe4_29_ : 1;
            bool xe4_30_ : 1;
            bool xe4_31_lightsDirty : 1;
            bool xe5_24_ : 1;
            bool xe5_25_ : 1;
            bool xe5_26_muted : 1;
            bool xe5_27_useInSortedLists : 1;
            bool xe5_28_callTouch : 1;
            bool xe6_26_inFluid : 1;
            u8 xe6_27_ : 3;
            bool xe6_30_enablePitchBend : 1;
            bool xe7_29_ : 1;
            bool xe7_30_ : 1;
            bool xe7_31_ : 1;
        };
        u32 dummy = 0;
    };
    void _CreateShadow();

public:
    enum class EFluidState
    {
    };

    enum class EScanState
    {
        Start,
        Processing,
        Done,
    };

    CActor(TUniqueId, bool, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
           const CMaterialList&, const CActorParameters&, TUniqueId);

    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    virtual void SetActive(bool active)
    {
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_ = true;
        xe7_29_ = true;
        CEntity::SetActive(active);
    }
    virtual void PreRender(CStateManager&, const zeus::CFrustum&) {}
    virtual void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    virtual void Render(const CStateManager&) const {}
    virtual bool CanRenderUnsorted(const CStateManager&) const { return false; }
    virtual void CalculateRenderBounds();
    virtual CHealthInfo* HealthInfo();
    virtual const CDamageVulnerability* GetDamageVulnerability() const;
    virtual const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                               const CDamageInfo&) const;
    virtual rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    virtual void Touch(CActor&, CStateManager&);
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetHomingPosition(const CStateManager&, float) const;
    virtual zeus::CVector3f GetScanObjectIndicatorPosition(const CStateManager&);
    virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                   CWeaponMode&, int);
    virtual void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
    virtual void OnScanStateChanged(EScanState, CStateManager&);
    virtual zeus::CAABox GetSortingBounds(const CStateManager&) const;
    virtual void DoUserAnimEvent(CStateManager&, CInt32POINode&, EUserEventType);

    void RemoveEmitter();
    const zeus::CTransform& GetTransform() const { return x34_transform; }
    const zeus::CVector3f& GetTranslation() const { return x34_transform.origin; }
    const zeus::CTransform GetScaledLocatorTransform(const std::string& segName) const;
    const zeus::CTransform GetLocatorTransform(const std::string& segName) const;
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, CStateManager&);

    void CreateShadow(bool);
    void SetCallTouch(bool callTouch);
    bool GetCallTouch() const;
    void SetUseInSortedLists(bool use);
    bool GetUseInSortedLists() const;
    const CMaterialFilter& GetMaterialFilter() const { return x70_materialFilter; }
    void SetMaterialFilter(const CMaterialFilter& filter) { x70_materialFilter = filter; }
    const CMaterialList& GetMaterialList() const { return x68_material; }
    void SetInFluid(bool in, TUniqueId uid);
    bool HasModelData() const;
    const CSfxHandle* GetSfxHandle() const;
    void SetSfxPitchBend(s32);
    void SetRotation(const zeus::CQuaternion& q);
    void SetTranslation(const zeus::CVector3f& tr);
    void SetTransform(const zeus::CTransform& tr);
    void SetAddedToken(u32 tok);
    float GetPitch() const;
    float GetYaw() const;
    const CModelData* GetModelData() const { return x64_modelData.get(); }
    CModelData* ModelData() { return x64_modelData.get(); }
    void EnsureRendered(const CStateManager&);
    void EnsureRendered(const CStateManager&, const zeus::CVector3f&, const zeus::CVector3f&);
    SAdvancementDeltas UpdateAnimation(float, CStateManager&, bool);
    void SetActorLights(std::unique_ptr<CActorLights>);
    bool GetE7_29() const  { return xe7_29_; }
};
}

#endif // __URDE_CACTOR_HPP__
