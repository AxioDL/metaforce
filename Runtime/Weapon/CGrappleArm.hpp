#ifndef __URDE_CGRAPPLEARM_HPP__
#define __URDE_CGRAPPLEARM_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"
#include "CStateManager.hpp"
#include "CGunController.hpp"

namespace urde
{

class CGrappleArm
{
public:
    enum class EArmState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten
    };
private:
    CModelData x0_modelData;
    zeus::CTransform x220_xf;
    std::unique_ptr<CGunController> x328_gunController;
    EArmState x334_animState;
    union
    {
        struct
        {
            bool x3b2_24_active : 1;
            bool x3b2_25_beamActive : 1;
            bool x3b2_27_armMoving : 1;
            bool x3b2_28_isGrappling : 1;
            bool x3b2_29_suitLoading : 1;
        };
        u32 _dummy = 0;
    };

public:
    explicit CGrappleArm(const zeus::CVector3f& vec);
    void AsyncLoadSuit(CStateManager& mgr);
    void SetTransform(const zeus::CTransform& xf) { x220_xf = xf; }
    const zeus::CTransform& GetTransform() const { return x220_xf; }
    void SetAnimState(EArmState state);
    EArmState GetAnimState() const { return x334_animState; }
    bool GetActive() const { return x3b2_24_active; }
    bool BeamActive() const { return x3b2_25_beamActive; }
    bool IsArmMoving() const { return x3b2_27_armMoving; }
    bool IsGrappling() const { return x3b2_28_isGrappling; }
    bool IsSuitLoading() const { return x3b2_29_suitLoading; }
    void Activate(bool);
    void GrappleBeamDisconnected();
    void GrappleBeamConnected();
    void RenderGrappleBeam(const CStateManager& mgr, const zeus::CVector3f& pos);
    void TouchModel(const CStateManager& mgr) const;
    void Update(float grappleSwingT, float dt, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void EnterStruck(CStateManager& mgr, float angle, bool attack, bool b2);
    void EnterIdle(CStateManager& mgr);
    void ReturnToDefault(CStateManager& mgr, float f1, bool b1);
    CGunController* GunController() { return x328_gunController.get(); }
};

}

#endif // __URDE_CGRAPPLEARM_HPP__
