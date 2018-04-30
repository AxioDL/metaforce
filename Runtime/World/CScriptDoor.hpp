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
        None = -1,
        Open,
        Close,
        Ready,
    };

    float x258_;
    float x25c_;
    EDoorAnimType x260_doorState = EDoorAnimType::Open;
    zeus::CAABox x264_;
    TUniqueId x27c_partner1 = kInvalidUniqueId;
    TUniqueId x27e_partner2 = kInvalidUniqueId;
    TUniqueId x280_ = kInvalidUniqueId;
    TUniqueId x282_dockId = kInvalidUniqueId;
    zeus::CAABox x284_modelBounds;

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
            bool x2a8_24_ : 1;
            bool x2a8_25_ : 1;
            bool x2a8_26_useConservativeCameraDistance : 1;
            bool x2a8_27_ : 1;
            bool x2a8_28_ : 1;
            bool x2a8_29_ballDoor : 1;
            bool x2a8_30_ : 1;
        };
        u32 dummy2 = 0;
    };
public:
    CScriptDoor(TUniqueId, std::string_view name, const CEntityInfo& info,
                const zeus::CTransform&, CModelData&&, const CActorParameters&,
                const zeus::CVector3f&, const zeus::CAABox&,
                bool active, bool open, bool, float, bool ballDoor);

    zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const;
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void Think(float, CStateManager& mgr);
    void AddToRenderer(const zeus::CFrustum&, CStateManager& mgr);
    void Render(const CStateManager&) {}
    void ForceClosed(CStateManager&);
    bool IsConnectedToArea(const CStateManager& mgr, TAreaId area);
    void OpenDoor(TUniqueId, CStateManager&);
    u32 GetDoorOpenCondition(CStateManager& mgr);
    void SetDoorAnimation(EDoorAnimType);
    std::experimental::optional<zeus::CAABox> GetProjectileBounds() const;
};

}

#endif // __URDE_CSCRIPTDOOR_HPP__
