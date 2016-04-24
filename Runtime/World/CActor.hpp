#ifndef __URDE_CACTOR_HPP__
#define __URDE_CACTOR_HPP__

#include "CEntity.hpp"
#include "zeus/zeus.hpp"
#include "Collision/CMaterialList.hpp"
#include "Character/CModelData.hpp"

namespace urde
{

enum class ECollisionResponseType
{
    Unknown12 = 0xC,
};

class CActorParameters;
class CWeaponMode;
class CHealthInfo;
class CDamageVulnerability;
class CLightParameters;

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
    CMaterialList x68_;
    CMaterialList x70_;
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

    virtual void Accept(IVisitor&) {}/*= 0;*/
    virtual void AddToRenderer(const zeus::CFrustum&, CStateManager&) {}
    virtual void Render(CStateManager&) {}
    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    virtual void SetActive(bool active)
    {
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_ = true;
        xe7_29_ = true;
        CEntity::SetActive(active);
    }

    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float)
    { return x34_transform.m_origin; }

    virtual bool ValidAimTarget() { return true; }
    virtual bool ValidOrbitTarget() { return true; }
    virtual bool GetOrbitDistanceCheck() { return true; }
    virtual zeus::CVector3f GetOrbitPosition(const CStateManager&)
    { return x34_transform.m_origin; }

    virtual ECollisionResponseType GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&, int) { return ECollisionResponseType::Unknown12; }
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
};

}

#endif // __URDE_CACTOR_HPP__
