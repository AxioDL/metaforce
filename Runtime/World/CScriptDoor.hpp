#ifndef __URDE_CSCRIPTDOOR_HPP__
#define __URDE_CSCRIPTDOOR_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptDoor : public CPhysicsActor
{
public:
    enum class EDoorAnimType
    {
        Zero,
        One,
        Two,
        Three
    };

    float x25c_;
    EDoorAnimType x260_doorState = EDoorAnimType::Zero;
    zeus::CAABox x264_;
    TUniqueId x27c_otherId = kInvalidUniqueId;
    TUniqueId x282_dockId = kInvalidUniqueId;

    zeus::CVector3f x29c_;
    union
    {
        struct
        {
            bool x2a0_25_ : 1;
            bool x2a0_26_ : 1;
        };
        u32 dummy1 = 0;
    };

    union
    {
        struct
        {
            bool x2a8_25_ : 1;
            bool x2a8_26_ : 1;
            bool x2a8_27_ : 1;
            bool x2a8_29_ : 1;
            bool x2a8_30_ : 1;
        };
        u32 dummy2 = 0;
    };
public:
    CScriptDoor(TUniqueId, const std::string& name, const CEntityInfo& info,
                const zeus::CTransform&, CModelData&&, const CActorParameters&,
                const zeus::CVector3f&, const zeus::CAABox&,
                bool active, bool material, bool, float, bool ballDoor);

    zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const;
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void ForceClosed(CStateManager&);
    bool IsConnectedToArea(const CStateManager& mgr, TAreaId area);
    void OpenDoor(TUniqueId, CStateManager&);
    u32 GetDoorOpenCondition(CStateManager& mgr);
    void SetDoorAnimation(EDoorAnimType);
};

}

#endif // __URDE_CSCRIPTDOOR_HPP__
