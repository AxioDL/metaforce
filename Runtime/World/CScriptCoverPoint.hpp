#pragma once

#include "CActor.hpp"

namespace urde
{
class CScriptCoverPoint : public CActor
{
    union
    {
        struct
        {
            bool xe8_26_landHere : 1;
            bool xe8_27_wallHang : 1;
            bool xe8_28_stay : 1;
        };
        u32 xe8_flags;
    };

    float xec_cosHorizontalAngle;
    float xf0_sinVerticalAngle;
    float xf4_coverTime;
    union
    {
        struct
        {
            bool xf8_24_crouch : 1;
            bool xf8_25_inUse : 1;
        };
        u32 xf8_flags = 0;
    };
    TUniqueId xfa_occupant = kInvalidUniqueId;
    TUniqueId xfc_retreating = kInvalidUniqueId;
    std::experimental::optional<zeus::CAABox> x100_touchBounds;
    float x11c_timeLeft = 0.f;
public:
    CScriptCoverPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                      zeus::CTransform xf, bool active, u32 flags, bool crouch, float horizontalAngle, float verticalAngle, float coverTime);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const {}
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void Render(const CStateManager &) const {}
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void SetInUse(bool inUse);
    bool GetInUse(TUniqueId uid) const;
    bool ShouldLandHere() const { return xe8_26_landHere; }
    bool ShouldWallHang() const { return xe8_27_wallHang; }
    bool ShouldStay() const { return xe8_28_stay; }
    bool ShouldCrouch() const { return xf8_24_crouch; }
    bool Blown(const zeus::CVector3f& pos) const;
    float GetSinSqVerticalAngle() const;
    float GetCosHorizontalAngle() const { return xec_cosHorizontalAngle; }
    u32 GetAttackDirection() const { return xe8_flags; }
    void Reserve(TUniqueId id) { xfa_occupant = id; }
};
}

