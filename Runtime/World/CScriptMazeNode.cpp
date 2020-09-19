#include "Runtime/World/CScriptMazeNode.hpp"

#include "Runtime/Character/CModelData.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

std::array<s32, 300> CScriptMazeNode::sMazeSeeds{};

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 w1, s32 w2, s32 w3,
                                 const zeus::CVector3f& actorPos, const zeus::CVector3f& triggerPos,
                                 const zeus::CVector3f& effectPos)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_(w1)
, xec_(w1)
, xf0_(w2)
, x100_actorPos(actorPos)
, x110_triggerPos(triggerPos)
, x120_effectPos(effectPos) {}

void CScriptMazeNode::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptMazeNode::LoadMazeSeeds() {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("DUMB_MazeSeeds");
  const u32 resSize = g_ResFactory->ResourceSize(*tag);
  const std::unique_ptr<u8[]> buf = g_ResFactory->LoadResourceSync(*tag);
  CMemoryInStream in(buf.get(), resSize);
  for (auto& seed : sMazeSeeds) {
    seed = in.readInt32Big();
  }
}

void CScriptMazeNode::GenerateObjects(CStateManager& mgr) {
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state != EScriptObjectState::MaxReached || conn.x4_msg != EScriptObjectMessage::Activate) {
      continue;
    }
    const auto* ent = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId));
    TCastToConstPtr<CScriptEffect> scriptEffect{ent};
    TCastToConstPtr<CScriptActor> scriptActor{ent};
    TCastToConstPtr<CScriptTrigger> scriptTrigger{ent};
    if ((scriptEffect || scriptActor || scriptTrigger) && (!scriptEffect || !x13c_25_)) {
      bool wasGeneratingObject = mgr.GetIsGeneratingObject();
      mgr.SetIsGeneratingObject(true);
      const auto genObj = mgr.GenerateObject(conn.x8_objId);
      mgr.SetIsGeneratingObject(wasGeneratingObject);
      auto* actor = static_cast<CActor*>(mgr.ObjectById(genObj.second));
      if (actor != nullptr) {
        mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::Activate);
        if (scriptEffect) {
          actor->SetTranslation(GetTranslation() + x120_effectPos);
          x11c_effectId = genObj.second;
        }
        if (scriptActor) {
          actor->SetTranslation(GetTranslation() + x100_actorPos);
          xfc_actorId = genObj.second;
        }
        if (scriptTrigger) {
          actor->SetTranslation(GetTranslation() + x110_triggerPos);
          x10c_triggerId = genObj.second;
        }
      }
    }
  }
}
} // namespace urde
