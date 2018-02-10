#include "CScriptColorModulate.hpp"
#include "Graphics/CModel.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptColorModulate::CScriptColorModulate(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                           const zeus::CColor& c1, const zeus::CColor& c2,
                                           EBlendMode bm, float f1, float f2,
                                           bool b1, bool b2, bool b3, bool b4, bool b5, bool active)
: CEntity(uid, info, active, name),
  x40_(c1),
  x44_(c2),
  x48_blendMode(bm),
  x4c_(f1),
  x50_(f2)
{
    x54_24_ = b1;
    x54_25_ = b2;
    x54_26_ = b3;
    x54_27_ = b4;
    x54_28_ = b5;
}

void CScriptColorModulate::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptColorModulate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr)
{
    CEntity::AcceptScriptMsg(msg, objId, mgr);
    return;
    if (!GetActive())
        return;

    if (msg == EScriptObjectMessage::Decrement)
    {
        if (x54_29_)
        {
            x38_ = x38_ == 0;
            x54_29_ = false;
            return;
        }
        else if (x54_30_)
        {
            if (x38_ == 0)
                x3c_ = 0.f;
            else
                x3c_ = -((x50_ * (x3c_ / x4c_)) - x50_);
        }
        else
            SetTargetFlags(mgr, CalculateFlags(x44_));

        x54_30_ = true;
        x38_ = 0;
    }
    else if (msg == EScriptObjectMessage::Increment)
    {
        if (x54_29_)
        {
            x38_ = x38_ == 0;
            x54_29_ = false;
            return;
        }
        else if (x54_30_)
        {
            if (x38_ == 0)
                x3c_ = 0.f;
            else
                x3c_ = -((x4c_ * (x3c_ / x50_)) - x4c_);
        }
        else
            SetTargetFlags(mgr, CalculateFlags(x40_));

        x54_30_ = true;
        x38_ = 0;
    }
}

void CScriptColorModulate::Think(float dt, CStateManager& mgr)
{
    return;
    if (!GetActive() || !x54_30_)
        return;

    x3c_ += dt;
    if (x38_ == 0)
    {
        float f2 = x4c_;
        float f1 = f2 - dt;
        if (std::fabs(f1) < 0.000001)
            f1 = 1.f;
        else
        {
            f1 = x3c_;
            f1 /= f2;
            if (f1 >= 1.f)
                f1 = 1.f;
        }

        zeus::CColor lerpedCol = zeus::CColor::lerp(x40_, x44_, f1);
        CModelFlags flags = CalculateFlags(lerpedCol);
        SetTargetFlags(mgr, flags);

        if (x3c_ <= x4c_)
            return;

        End(mgr);
    }
    else if (x38_ == 1)
    {
        float f2 = x50_;
        float f1 = f2 - dt;
        if (std::fabs(f1) < 0.000001)
            f1 = 1.f;
        else
        {
            f1 = x3c_;
            f1 /= f2;
            if (f1 >= 1.f)
                f1 = 1.f;
        }

        zeus::CColor lerpedCol = zeus::CColor::lerp(x40_, x44_, f1);
        CModelFlags flags = CalculateFlags(lerpedCol);
        SetTargetFlags(mgr, flags);

        if (x3c_ <= x50_)
            return;

        End(mgr);
    }
}

CModelFlags CScriptColorModulate::CalculateFlags(const zeus::CColor &) const
{
    return {};
}

void CScriptColorModulate::SetTargetFlags(CStateManager& stateMgr, const CModelFlags& flags)
{
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
            continue;

        auto search = stateMgr.GetIdListForScript(conn.x8_objId);
        for (auto it = search.first ; it != search.second ; ++it)
        {
            CEntity* ent = stateMgr.ObjectById(it->second);
            if (CActor* act = TCastToPtr<CActor>(ent))
                act->SetDrawFlags(flags);
        }
    }

    if (x34_ != kInvalidUniqueId)
    {
        CEntity* ent = stateMgr.ObjectById(x34_);
        if (CActor* act = TCastToPtr<CActor>(ent))
            act->SetDrawFlags(flags);
    }
}

void CScriptColorModulate::FadeOutHelper(CStateManager &, TUniqueId, float)
{
}

void CScriptColorModulate::FadeInHelper(CStateManager& mgr, TUniqueId uid, float f1)
{

}

void CScriptColorModulate::End(CStateManager& stateMgr)
{
    x3c_ = 0.f;
    if (x54_24_ && !x54_29_)
    {
        x54_29_ = true;
        x38_ = x38_ == 0;
        return;
    }

    x54_30_ = false;
    x54_29_ = false;
    if (x54_25_)
        SetTargetFlags(stateMgr, CModelFlags(0, 0, 3, zeus::CColor::skWhite));

    if (x55_24_)
        stateMgr.SendScriptMsgAlways(x34_, x8_uid, EScriptObjectMessage::Deactivate);

    CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, stateMgr, EScriptObjectMessage::None);

    if (!x54_31_)
        stateMgr.FreeScriptObject(GetUniqueId());
}
}
