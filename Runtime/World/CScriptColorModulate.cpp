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
    x54_26_depthEqual = b3;
    x54_27_depthUpdate = b4;
    x54_28_ = b5;
    x54_29_ = false;
    x54_30_ = false;
    x54_31_ = false;
    x55_24_ = false;
}

void CScriptColorModulate::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptColorModulate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr)
{
    CEntity::AcceptScriptMsg(msg, objId, mgr);

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

CModelFlags CScriptColorModulate::CalculateFlags(const zeus::CColor& col) const
{
    CModelFlags ret;
    if (x54_28_)
    {
        if (x48_blendMode == EBlendMode::Zero)
        {
            CModelFlags ret;
            ret.x0_blendMode = 5;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = (x54_29_ << 1) | (x54_27_depthUpdate << 0) | 3 | 8;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::One)
        {
            CModelFlags ret;
            ret.x0_blendMode = 7;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1 | 0x8;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::Two)
        {
            CModelFlags ret;
            ret.x0_blendMode = 8;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1 | 0x8;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::Three)
        {
            CModelFlags ret;
            ret.x0_blendMode = 1;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1 | 0x8;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::Four)
        {
            CModelFlags ret;
            ret.x0_blendMode = 2;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1 | 0x8;
            ret.x4_color = col;
        }
        else
        {
            ret.x2_flags = 3;
            ret.x4_color = zeus::CColor::skWhite;
        }
    }
    else
    {
        if (x48_blendMode == EBlendMode::Zero)
        {
            if (col == zeus::CColor::skWhite)
            {
                ret.x0_blendMode = 3;
                ret.x1_matSetIdx = 0;
                ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
                ret.x4_color = zeus::CColor::skWhite;
            }
            else
            {
                ret.x0_blendMode = 5;
                ret.x1_matSetIdx = 0;
                ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
                ret.x4_color = col;
            }
        }
        else if (x48_blendMode == EBlendMode::One)
        {
            ret.x0_blendMode = 7;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::Two)
        {
            ret.x0_blendMode = 8;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
            ret.x4_color = col;
        }
        else if (x48_blendMode == EBlendMode::Three)
        {
            if (col == zeus::CColor::skWhite)
            {
                ret.x0_blendMode = 3;
                ret.x1_matSetIdx = 0;
                ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
                ret.x4_color = zeus::CColor::skWhite;
            }
            else
            {
                ret.x0_blendMode = 1;
                ret.x1_matSetIdx = 0;
                ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
                ret.x4_color = col;
            }
        }
        else if (x48_blendMode == EBlendMode::Four)
        {
            ret.x0_blendMode = 2;
            ret.x1_matSetIdx = 0;
            ret.x2_flags = x54_26_depthEqual << 0 | x54_27_depthUpdate << 1;
            ret.x4_color = col;
        }
        else
        {
            ret.x2_flags = 3;
            ret.x4_color = zeus::CColor::skWhite;
        }
    }
    return ret;
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

    if (x34_parent != kInvalidUniqueId)
    {
        CEntity* ent = stateMgr.ObjectById(x34_parent);
        if (CActor* act = TCastToPtr<CActor>(ent))
            act->SetDrawFlags(flags);
    }
}

TUniqueId CScriptColorModulate::FadeOutHelper(CStateManager& mgr, TUniqueId parent, float dt)
{
    TAreaId aId = mgr.GetNextAreaId();
    if (const CEntity* ent = mgr.GetObjectById(parent))
        aId = ent->GetAreaIdAlways();

    TUniqueId ret = mgr.AllocateUniqueId();
    CScriptColorModulate* colMod = new CScriptColorModulate(ret, "", CEntityInfo(aId, CEntity::NullConnectionList), zeus::CColor(1.f, 1.f, 1.f, 0.f), zeus::CColor(1.f, 1.f, 1.f, 1.f), EBlendMode::Zero, dt, 0.f, false, false, true, true, false, true);
    mgr.AddObject(colMod);
    colMod->x34_parent = parent;
    colMod->x54_30_ = true;
    colMod->x54_31_ = true;
    colMod->x55_24_ = true;

    colMod->Think(0.f, mgr);
    return ret;
}

TUniqueId CScriptColorModulate::FadeInHelper(CStateManager& mgr, TUniqueId parent, float dt)
{
    TAreaId aId = mgr.GetNextAreaId();
    if (const CEntity* ent = mgr.GetObjectById(parent))
        aId = ent->GetAreaIdAlways();

    TUniqueId ret = mgr.AllocateUniqueId();
    CScriptColorModulate* colMod = new CScriptColorModulate(ret, "", CEntityInfo(aId, CEntity::NullConnectionList), zeus::CColor(1.f, 1.f, 1.f, 1.f), zeus::CColor(1.f, 1.f, 1.f, 0.f), EBlendMode::Zero, dt, 0.f, false, false, true, true, false, true);
    mgr.AddObject(colMod);
    colMod->x34_parent = parent;
    colMod->x54_30_ = true;
    colMod->x54_31_ = true;

    colMod->Think(0.f, mgr);
    return ret;
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
        stateMgr.SendScriptMsgAlways(x34_parent, x8_uid, EScriptObjectMessage::Deactivate);

    CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, stateMgr, EScriptObjectMessage::None);

    if (!x54_31_)
        stateMgr.FreeScriptObject(GetUniqueId());
}
}
