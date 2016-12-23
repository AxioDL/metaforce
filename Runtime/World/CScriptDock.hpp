#ifndef __URDE_CSCRIPTDOCK_HPP__
#define __URDE_CSCRIPTDOCK_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptDock : public CPhysicsActor
{
    friend class CScriptDoor;
    s32 x258_;
    s32 x25c_dock;
    TAreaId x260_area;
    u32 x264_;
    union
    {
        struct
        {
            bool x268_24_ : 1;
            bool x268_25_ : 1;
            bool x268_26_ : 1;
        };
        u8 dummy = 0;
    };
public:
    CScriptDock(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CVector3f position,
                const zeus::CVector3f& extent, s32, TAreaId, bool active, s32 w1, bool b1);

    TAreaId GetAreaId() const { return x260_area; }
    s32 GetDockId() const { return x25c_dock; }
    void SetDockReference(s32) {}
    void GetDockReference(s32) {}
    TAreaId GetCurrentConnectedAreaId(const CStateManager&) const;
    void UpdateAreaActivateFlags(CStateManager&);
    bool HasPointCrossedDock(const CStateManager&, const zeus::CVector3f&) const;
    void AreaLoaded(CStateManager&);
    void AreaUnloaded(CStateManager&);
    void SetLoadConnected(CStateManager&, bool);
};
}
#endif // __URDE_CSCRIPTDOCK_HPP__
