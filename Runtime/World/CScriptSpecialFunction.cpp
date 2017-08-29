#include "CScriptSpecialFunction.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"
#include "Audio/CSfxManager.hpp"
#include "TCastTo.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptSpecialFunction::CScriptSpecialFunction(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, ESpecialFunction func,
                                               const std::string& lcName, float f1, float f2, float f3, float f4,
                                               const zeus::CVector3f& vec, const zeus::CColor& col, bool active,
                                               const CDamageInfo& dInfo, CAssetId aId1, CAssetId aId2, CAssetId aId3,
                                               s16 sId1, s16 sId2, s16 sId3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_function(func)
, xec_locatorName(lcName)
, xfc_(f1)
, x100_(f2)
, x104_(f3)
, x108_(f4)
, x10c_(vec)
, x118_(col)
, x11c_damageInfo(dInfo)
, x170_(CSfxManager::TranslateSFXID(sId1))
, x172_(CSfxManager::TranslateSFXID(sId2))
, x174_(CSfxManager::TranslateSFXID(sId3))
, x1bc_(aId1)
, x1c0_(aId2)
, x1c4_(aId3)
{
    x1e4_26_ = true;
    if (xe8_function == ESpecialFunction::HUDTarget)
        x1c8_ = {{zeus::CVector3f(-1.f), zeus::CVector3f(1.f)}};
}

void CScriptSpecialFunction::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpecialFunction::Think(float, CStateManager&) {}

void CScriptSpecialFunction::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
}

void CScriptSpecialFunction::PreRender(CStateManager&, const zeus::CFrustum&) {}

void CScriptSpecialFunction::AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}

void CScriptSpecialFunction::Render(const CStateManager&) const {}

void CScriptSpecialFunction::SkipCinematic(CStateManager& stateMgr)
{
    SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
    stateMgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr)
{
    float deltaScale = dt * xfc_;

    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
            continue;
        if (TCastToPtr<CActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId)))
        {
            CModelData* mData = act->ModelData();
            if (mData && (mData->HasAnimData() || mData->HasNormalModel()))
            {
                zeus::CVector3f scale = mData->GetScale();

                if (deltaScale > 0.f)
                    scale = zeus::min(deltaScale + scale, {x100_});
                else
                    scale = zeus::max(deltaScale + scale, {x100_});

                mData->SetScale(scale);
            }
        }
    }
}

void CScriptSpecialFunction::ThinkSaveStation(float, CStateManager& mgr)
{
    if (x1e5_24_doSave && mgr.GetDeferredStateTransition() != EStateManagerTransition::SaveGame)
    {
        x1e5_24_doSave = false;
        if (mgr.GetInSaveUI())
            SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
        else
            SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
    }
}

bool CScriptSpecialFunction::ShouldSkipCinematic(CStateManager& stateMgr) const
{
    return g_GameState->SystemOptions().GetCinematicState(stateMgr.GetWorld()->IGetWorldAssetId(), GetEditorId());
}
}
