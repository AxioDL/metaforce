#ifndef __URDE_CACTOR_HPP__
#define __URDE_CACTOR_HPP__

#include "CEntity.hpp"
#include "zeus/zeus.hpp"

namespace urde
{

enum class ECollisionResponseType
{
    Unknown12 = 0xC,
};

enum class EMaterialTypes
{
};

class CModelData;
class CMaterialList;
class CActorParameters;
class CWeaponMode;

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
    float x40_unknown;
    float x50_unknown;
    float x60_unknown;
    u32   x68_unknown;
    u32   x6c_unknown;
    bool    xd0_b0_flags : 1;
    bool    xd0_b1_flags : 1;
    bool    xd0_b2_flags : 1;
public:
    CActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
           const zeus::CTransform&, const CModelData&, const CMaterialList&,
           const CActorParameters&, TUniqueId);

    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    virtual zeus::CVector3f GetAimPosition(const CStateManager&, float)
    { return zeus::CVector3f(x40_unknown, x50_unknown, x60_unknown); }

    virtual bool ValidAimTarget() { return true; }
    virtual bool ValidOrbitTarget() { return true; }
    virtual bool GetOrbitDistanceCheck() { return true; }
    virtual zeus::CVector3f GetOrbitPosition()
    { return zeus::CVector3f(x40_unknown, x50_unknown, x60_unknown); }

    virtual ECollisionResponseType GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&, int) { return ECollisionResponseType::Unknown12; }
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&){}
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&){ }
    void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&){ }
    void RemoveMaterial(EMaterialTypes, CStateManager&){ }
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&){ }
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&){}
    void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&){ }
    void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&){ }
    void AddMaterial(EMaterialTypes, CStateManager&){ }

    virtual void SetActive(bool active)
    {
        //xd0_flags |= (Unknown5 | Unknown6 | Unknown7);
        CEntity::SetActive(active);
    }

    virtual void SetCallTouch(bool callTouch)
    {
        //xd1_flags = Unknown6
    }
};

}

#endif // __URDE_CACTOR_HPP__
