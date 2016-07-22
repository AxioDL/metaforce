#ifndef __URDE_CACTOR_HPP__
#define __URDE_CACTOR_HPP__

#include "CEntity.hpp"
#include "Graphics/CGraphics.hpp"
#include "Audio/CSfxHandle.hpp"
#include "zeus/zeus.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Character/CModelData.hpp"
#include "Particle/CCollisionResponseData.hpp"

namespace urde
{

class CActorParameters;
class CWeaponMode;
class CHealthInfo;
class CDamageVulnerability;
class CLightParameters;
class CSfxHandle;
class CSimpleShadow;

class CActor : public CEntity
{
protected:
    zeus::CTransform x34_transform;
    std::unique_ptr<CModelData> x64_modelData;
    CMaterialList x68_material;
    CMaterialFilter x70_;
    s16 x88_sfxId = -1;
    std::unique_ptr<CSfxHandle> x8c_sfxHandle;
    //std::unique_ptr<CSimpleShadow> x94_simpleShadow;
    zeus::CAABox x9c_aabox;
    u32 xb8_ = 0;
    float xbc_time = 0.f;
    s32 xc0_ = 0;
    TUniqueId xc4_fluidId = kInvalidUniqueId;
    TUniqueId xc6_ = kInvalidUniqueId;
    u8 xd4_ = 0x7F;
    union
    {
        struct
        {
            bool xe4_27_ : 1;
            bool xe4_28_ : 1;
            bool xe4_29_ : 1;
            bool xe4_30_ : 1;
        };
        u8 _dummy1 = 0;
    };

    union
    {
        struct
        {
            bool xe5_0_opaque : 1;
            bool xe5_26_muted : 1;
            bool xe5_27_useInSortedLists : 1;
            bool xe5_28_callTouch : 1;
        };
        u8 _dummy2 = 0;
    };
    union
    {
        struct
        {
            bool xe6_26_inFluid : 1;
            bool xe6_30_enablePitchBend : 1;
        };
        u8 _dummy3 = 0;
    };
    union
    {
        struct
        {
            bool xe7_29_ : 1;
        };
        u8 _dummy4 = 0;
    };
public:
    CActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
           const zeus::CTransform&, CModelData&&, const CMaterialList&,
           const CActorParameters&, TUniqueId);

    const zeus::CTransform& GetTransform() const {return x34_transform;}

    virtual void AddToRenderer(const zeus::CFrustum&, CStateManager&) {}
    virtual void Render(CStateManager&) {}
    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    virtual void SetActive(bool active)
    {
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_ = true;
        xe7_29_ = true;
        CEntity::SetActive(active);
    }

    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float)
    { return x34_transform.origin; }

    virtual bool ValidAimTarget() { return true; }
    virtual bool ValidOrbitTarget() { return true; }
    virtual bool GetOrbitDistanceCheck() { return true; }
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&);

    void RemoveEmitter();

    virtual rstl::optional_object<zeus::CAABox> GetTouchBounds() const { return {} ; }
    virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&, int);

    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, CStateManager&);

    void SetCallTouch(bool callTouch);

    bool GetCallTouch() const;

    void SetUseInSortedList(bool use);

    bool GetUseInSortedLists() const;

    const CMaterialFilter& GetMaterialFilter() const { return x70_; }

    void SetInFluid(bool in, TUniqueId uid);

    bool HasModelData() const;
    const CSfxHandle* GetSfxHandle() const;
    void SetSfxPitchBend(s32);
};

}

#endif // __URDE_CACTOR_HPP__
