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

void CScriptColorModulate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptColorModulate::Think(float, CStateManager &)
{
}

zeus::CColor CScriptColorModulate::CalculateFlags(const zeus::CColor &) const
{
    return {};
}

void CScriptColorModulate::SetTargetFlags(CStateManager &, const CModelFlags &)
{
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
        x38_ = 0;
        return;
    }

    x54_30_ = false;
    x54_29_ = false;
    if (x54_25_)
        SetTargetFlags(stateMgr, CModelFlags(0, 0, 3, zeus::CColor::skWhite));

    if (x54_24_)
        stateMgr.SendScriptMsg(x8_uid, x34_, EScriptObjectMessage::Deactivate);

    CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, stateMgr, EScriptObjectMessage::None);

    if (x54_31_)
        stateMgr.FreeScriptObject(x8_uid);
}
}
