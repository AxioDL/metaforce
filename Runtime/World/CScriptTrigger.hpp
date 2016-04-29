#ifndef __URDE_CSCRIPTTRIGGER_HPP__
#define __URDE_CSCRIPTTRIGGER_HPP__

#include "CActor.hpp"
#include "CDamageInfo.hpp"

namespace urde
{
class CScriptTrigger : public CActor
{
public:
    class CObjectTracker
    {
        TUniqueId x0_id;
    public:
        CObjectTracker(TUniqueId id)
            : x0_id(id)
        {}

        TUniqueId GetObjectId() const { return x0_id; }
    };

private:
    std::list<CObjectTracker> xe8_inhabitants; /* Used to be CObjectTracker */
    CDamageInfo x100_damageInfo;
    zeus::CVector3f x11c_forceField;
    float x128_forceMagnitude;
    u32 x12c_flags;
    zeus::CAABox x130_bounds;

    union
    {
        struct
        {
            bool x148_24_ : 1;
            bool x148_25_ : 1;
            bool x148_26_ : 1;
            bool x148_27_ : 1;
            bool x148_28_ : 1;
            bool x148_29_ : 1;
        };
        u8 dummy = 0;
    };
public:
    CScriptTrigger(TUniqueId, const std::string& name, const CEntityInfo& info,
                   const zeus::CVector3f& pos, const zeus::CAABox&,
                   const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                   u32 triggerFlags, bool, bool, bool);

    virtual void InhabitantRejected(CActor&, CStateManager&) {}
    virtual void InhabitantExited(CActor&, CStateManager&) {}
    virtual void InhabitantIdle(CActor&, CStateManager&) {}
    virtual void InhabitantAdded(CActor&, CStateManager&) {}
    CObjectTracker* FindInhabitant(TUniqueId);
    void UpdateInhabitants(CStateManager&);
    const std::list<CObjectTracker>& GetInhabitants() const;
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    zeus::CAABox GetTriggerBoundsWR() const;
};

}

#endif // __URDE_CSCRIPTTRIGGER_HPP__
