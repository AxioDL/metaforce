#ifndef __URDE_CACTOR_HPP__
#define __URDE_CACTOR_HPP__

#include "CEntity.hpp"
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
class CActor : public CEntity
{
protected:
    enum class Flags
    {
        Unknown5 = (1 << 5),
        Unknown6 = (1 << 6),
        Unknown7 = (1 << 7),
    };

    zeus::CTransform x34_transform;
    std::unique_ptr<CModelData> x64_modelData;
    CMaterialList x68_material;
    CMaterialFilter x70_;
    s16 x88_sfxId;
    std::unique_ptr<CSfxHandle> x8c_sfxHandle;
    TUniqueId xc6_ = kInvalidUniqueId;
    float xbc_time;
    union
    {
        struct
        {
            bool xe4_27_ : 1;
            bool xe4_28_ : 1;
            bool xe4_29_ : 1;
            bool xe4_30_ : 1;
            bool xe5_0_opaque : 1;
            bool xe5_26_muted : 1;
            bool xe5_27_useInSortedLists : 1;
            bool xe5_28_callTouch : 1;
        };
        u32 dummy1 = 0;
    };

    union
    {
        struct
        {
            bool xe7_29_ : 1;
        };
        u32 dummy2 = 0;
    };
public:
    CActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
           const zeus::CTransform&, CModelData&&, const CMaterialList&,
           const CActorParameters&, TUniqueId);

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
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&)
    { return x34_transform.origin; }

    void RemoveEmitter();

    virtual std::experimental::optional<zeus::CAABox> GetTouchBounds() const { return {} ; }

    virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&, int) { return EWeaponCollisionResponseTypes::Unknown13; }
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void RemoveMaterial(EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
    void AddMaterial(EMaterialTypes, CStateManager&);

    void SetCallTouch(bool callTouch)
    {
        xe5_28_callTouch = callTouch;
    }

    bool GetCallTouch() const
    {
        return xe5_28_callTouch;
    }

    void SetUseInSortedList(bool use)
    {
        xe5_27_useInSortedLists = use;
    }

    bool GetUseInSortedLists() const
    {
        return xe5_27_useInSortedLists;
    }

    const CMaterialFilter& GetMaterialFilter() const { return x70_; }

    bool HasModelData() const;
};

}

#endif // __URDE_CACTOR_HPP__
