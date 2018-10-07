#pragma once

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptDock : public CPhysicsActor
{
    enum EDockState
    {
        Idle,
        PlayerTouched,
        EnterNextArea,
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
    CScriptDock(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CVector3f position,
                const zeus::CVector3f& extent, s32 dock, TAreaId area, bool active,
                s32 dockReferenceCount, bool loadConnected);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    void CleanUp() {}
    zeus::CPlane GetPlane(const CStateManager&) const;
    TAreaId GetAreaId() const { return x260_area; }
    s32 GetDockId() const { return x25c_dock; }
    void SetDockReference(CStateManager& mgr, s32);
    s32 GetDockReference(const CStateManager& mgr) const;
    TAreaId GetCurrentConnectedAreaId(const CStateManager&) const;
    void UpdateAreaActivateFlags(CStateManager&);
    bool HasPointCrossedDock(const CStateManager&, const zeus::CVector3f&) const;
    void AreaLoaded(CStateManager&);
    void AreaUnloaded(CStateManager&);
    void SetLoadConnected(CStateManager&, bool);
};
}
