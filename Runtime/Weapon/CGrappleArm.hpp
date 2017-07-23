#ifndef __URDE_CGRAPPLEARM_HPP__
#define __URDE_CGRAPPLEARM_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"

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
    };
private:
    CModelData x0_modelData;
    zeus::CTransform x220_xf;
    EArmState x334_animState;
    union
    {
        struct
        {
            bool x3b2_25_beamActive : 1;
            bool x3b2_27_armMoving : 1;
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
    bool BeamActive() const { return x3b2_25_beamActive; }
    bool IsArmMoving() const { return x3b2_27_armMoving; }
    void Activate(bool);
    void GrappleBeamDisconnected();
    void GrappleBeamConnected();
};

}

#endif // __URDE_CGRAPPLEARM_HPP__
