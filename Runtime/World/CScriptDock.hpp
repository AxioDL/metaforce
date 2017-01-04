#ifndef __URDE_CSCRIPTDOCK_HPP__
#define __URDE_CSCRIPTDOCK_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptDock : public CPhysicsActor
{
    enum EDockState
    {
        Zero,
        PlayerTouched,
        Two,
        Three
    };

    friend class CScriptDoor;
    s32 x258_dockReferenceCount;
    s32 x25c_dock;
    TAreaId x260_area;
    EDockState x264_dockState = EDockState::Three;
    union {
        struct
        {
            bool x268_24_dockReferenced : 1;
            bool x268_25_loadConnected : 1;
            bool x268_26_areaPostConstructed : 1;
        };
        u8 dummy = 0;
    };

public:
    CScriptDock(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CVector3f position,
                const zeus::CVector3f& extent, s32, TAreaId, bool active, s32 w1, bool b1);

    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    void CleanUp() {}
    zeus::CPlane GetPlane(const CStateManager&) const;
    TAreaId GetDestinationAreaId() const { return x260_area; }
    s32 GetDockId() const { return x25c_dock; }
    void SetDockReference(CStateManager& mgr, s32);
    s32 GetDockReference(CStateManager& mgr) const;
    TAreaId GetCurrentConnectedAreaId(const CStateManager&) const;
    void UpdateAreaActivateFlags(CStateManager&);
    bool HasPointCrossedDock(const CStateManager&, const zeus::CVector3f&) const;
    void AreaLoaded(CStateManager&);
    void AreaUnloaded(CStateManager&);
    void SetLoadConnected(CStateManager&, bool);
};
}
#endif // __URDE_CSCRIPTDOCK_HPP__
