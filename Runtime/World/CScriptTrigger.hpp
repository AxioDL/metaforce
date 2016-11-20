#ifndef __URDE_CSCRIPTTRIGGER_HPP__
#define __URDE_CSCRIPTTRIGGER_HPP__

#include "CActor.hpp"
#include "CDamageInfo.hpp"

namespace urde
{

// TODO - Phil: Figure out what each of the DetectProjectiles actually mean
enum class ETriggerFlags : u32
{
    DetectPlayer = (1 << 0),
    DetectAI = (1 << 1),
    DetectProjectiles1 = (1 << 2),
    DetectProjectiles2 = (1 << 3),
    DetectProjectiles3 = (1 << 4),
    DetectProjectiles4 = (1 << 5),
    DetectBombs = (1 << 6),
    Unknown1 = (1 << 7),
    DetectProjectiles5 = (1 << 8),
    DetectProjectiles6 = (1 << 9),
    DetectProjectiles7 = (1 << 10),
    KillOnEnter = (1 << 11),
    DetectMorphedPlayer = (1 << 12),
    ApplyForce = (1 << 13),
    DetectPlayerIfInside = (1 << 14),
    Unknown2 = (1 << 15),
    DetectUnmorphedPlayer = (1 << 16),
    BlockEnvironmentalEffects = (1 << 17)
};
ENABLE_BITWISE_ENUM(ETriggerFlags)

class CScriptTrigger : public CActor
{
public:
    class CObjectTracker
    {
        TUniqueId x0_id;

    public:
        CObjectTracker(TUniqueId id) : x0_id(id) {}

        TUniqueId GetObjectId() const { return x0_id; }
        bool operator==(const CObjectTracker& other) { return x0_id == other.x0_id; }
    };

protected:
    std::list<CObjectTracker> xe8_inhabitants;
    CDamageInfo x100_damageInfo;
    zeus::CVector3f x11c_forceField;
    float x128_forceMagnitude;
    ETriggerFlags x12c_flags;
    zeus::CAABox x130_bounds;

    union {
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
    CScriptTrigger(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CVector3f& pos,
                   const zeus::CAABox&, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                   ETriggerFlags triggerFlags, bool, bool, bool);

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
