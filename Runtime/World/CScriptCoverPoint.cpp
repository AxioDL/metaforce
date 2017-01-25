#include "CScriptCoverPoint.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptCoverPoint::CScriptCoverPoint(TUniqueId uid, const std::string &name, const CEntityInfo &info,
                                     zeus::CTransform xf, bool active, u32 flags, bool crouch, float horizontalAngle, float verticalAngle, float coverTime)
    : CActor(uid, active, name, info, xf, CModelData::CModelDataNull(),
             CMaterialList(EMaterialTypes::Unknown), CActorParameters::None(), kInvalidUniqueId),
      xe8_flags(flags),
      xf4_coverTime(coverTime)
{
    xf8_24_crouch = crouch;
    xec_cosHorizontalAngle = std::cos(zeus::degToRad(horizontalAngle) * 0.5f);
    xf0_sinVerticalAngle = std::sin(zeus::degToRad(verticalAngle) * 0.5f);
    x100_touchBounds.emplace(xf.origin, xf.origin);
}

void CScriptCoverPoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCoverPoint::Think(float delta, CStateManager&)
{
    if (x11c_timeLeft <= 0.f)
        return;
    x11c_timeLeft -= delta;
}

rstl::optional_object<zeus::CAABox> CScriptCoverPoint::GetTouchBounds() const
{
    if (x100_touchBounds)
        return x100_touchBounds;

    return {};
}

void CScriptCoverPoint::SetInUse(bool inUse)
{
    xf8_25_inUse = inUse;
    if (inUse)
        x11c_timeLeft = xf4_coverTime;
}

bool CScriptCoverPoint::GetInUse(TUniqueId uid) const
{
    if (xf8_25_inUse || x11c_timeLeft > 0.f)
        return true;

    if (xfa_occupant == kInvalidUniqueId || uid == kInvalidUniqueId || xfa_occupant == uid)
        return false;

    return true;
}


bool CScriptCoverPoint::Blown(const zeus::CVector3f& point) const
{
    if (!x30_24_active)
        return true;

    if (ShouldWallHang())
    {
        zeus::CVector3f posDif = point - x34_transform.origin;
        posDif *= (1.0 / posDif.magnitude());
        zeus::CVector3f normDif = posDif.normalized();

        zeus::CVector3f frontVec = x34_transform.frontVector();
        frontVec.normalize();

        if (frontVec.dot(normDif) <= GetCosHorizontalAngle() || (posDif.z * posDif.z) >= GetSinSqVerticalAngle())
            return true;
    }
    return false;
}

float CScriptCoverPoint::GetSinSqVerticalAngle() const
{
    return xf0_sinVerticalAngle * xf0_sinVerticalAngle;
}

float CScriptCoverPoint::GetCosHorizontalAngle() const
{
    return xec_cosHorizontalAngle;
}

bool CScriptCoverPoint::ShouldLandHere() const
{
    return xe8_26_landHere;
}

bool CScriptCoverPoint::ShouldWallHang() const
{
    return xe8_27_wallHang;
}

bool CScriptCoverPoint::ShouldStay() const
{
    return xe8_28_stay;
}

bool CScriptCoverPoint::ShouldCrouch() const
{
    return xf8_24_crouch;
}

u32 CScriptCoverPoint::GetAttackDirection() const
{
    return xe8_flags;
}

void CScriptCoverPoint::Reserve(TUniqueId id)
{
    xfa_occupant = id;
}

void CScriptCoverPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);

    if (msg == EScriptObjectMessage::InternalMessage14)
    {
        for (const SConnection& con : x20_conns)
            if (con.x0_state == EScriptObjectState::Retreat)
            {
                xfc_retreating = mgr.GetIdForScript(con.x8_objId);
                break;
            }
    }
}


}
