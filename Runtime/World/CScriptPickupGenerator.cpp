#include "CScriptPickupGenerator.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path
#include "CStateManager.hpp"
#include "CScriptPickup.hpp"
#include "CWallCrawlerSwarm.hpp"

namespace urde {

CScriptPickupGenerator::CScriptPickupGenerator(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CVector3f& pos, float frequency, bool active)
: CEntity(uid, info, active, name), x34_position(pos), x40_frequency(frequency) {
  ResetDelayTimer();
}

void CScriptPickupGenerator::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPickupGenerator::ResetDelayTimer() {
  if (x40_frequency > 0.f)
    x44_delayTimer += 100.f / x40_frequency;
  else
    x44_delayTimer = FLT_MAX;
}

void CScriptPickupGenerator::GetGeneratorIds(CStateManager& mgr, TUniqueId sender,
                                             std::vector<TUniqueId>& idsOut) const {
  idsOut.reserve(std::max(size_t(1), GetConnectionList().size()));
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Zero && conn.x4_msg == EScriptObjectMessage::Follow) {
      TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
      if (id != kInvalidUniqueId) {
        if (const CEntity* ent = mgr.GetObjectById(id)) {
          if (ent->GetActive())
            idsOut.push_back(id);
        }
      }
    }
  }
  if (idsOut.empty())
    idsOut.push_back(sender);
}

float CScriptPickupGenerator::GetPickupTemplates(CStateManager& mgr,
                                                 std::vector<std::pair<float, TEditorId>>& idsOut) const {
  float totalPossibility = 0.f;
  CPlayerState& pState = *mgr.GetPlayerState();
  idsOut.reserve(GetConnectionList().size());
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Zero && conn.x4_msg == EScriptObjectMessage::Activate) {
      TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
      if (id != kInvalidUniqueId) {
        if (TCastToConstPtr<CScriptPickup> pickup = mgr.GetObjectById(id)) {
          CPlayerState::EItemType item = pickup->GetItem();
          float possibility = pickup->GetPossibility();
          float multiplier = 1.f;
          bool doAlways = false;
          bool doThirtyPerc = false;
          switch (item) {
          case CPlayerState::EItemType::Missiles:
            if (pState.HasPowerUp(CPlayerState::EItemType::Missiles)) {
              if (pState.GetItemAmount(CPlayerState::EItemType::Missiles) <
                  pState.GetItemCapacity(CPlayerState::EItemType::Missiles))
                doAlways = true;
              else
                doThirtyPerc = true;
            }
            break;
          case CPlayerState::EItemType::PowerBombs:
            if (pState.HasPowerUp(CPlayerState::EItemType::PowerBombs)) {
              if (pState.GetItemAmount(CPlayerState::EItemType::PowerBombs) <
                  pState.GetItemCapacity(CPlayerState::EItemType::PowerBombs)) {
                doAlways = true;
                if (pState.GetItemAmount(CPlayerState::EItemType::PowerBombs) < 2 &&
                  possibility >= 10.f && possibility < 25.f)
                  multiplier = 2.f;
              } else {
                doThirtyPerc = true;
              }
            }
            break;
          case CPlayerState::EItemType::HealthRefill:
            if (pState.GetHealthInfo().GetHP() < pState.CalculateHealth())
              doAlways = true;
            else
              doThirtyPerc = true;
            break;
          default:
            doAlways = true;
            break;
          }
          bool thirtyPercTest = mgr.GetActiveRandom()->Float() < 0.3f;
          if ((doAlways || (doThirtyPerc && thirtyPercTest)) && possibility > 0.f) {
            totalPossibility += possibility * multiplier;
            idsOut.push_back(std::make_pair(possibility, conn.x8_objId));
          }
        }
      }
    }
  }
  return totalPossibility;
}

void CScriptPickupGenerator::GeneratePickup(CStateManager& mgr, TEditorId templateId, TUniqueId generatorId) const {
  CEntity* pickupTempl = mgr.ObjectById(mgr.GetIdForScript(templateId));
  CEntity* generator = mgr.ObjectById(generatorId);
  if (pickupTempl && generator) {
    bool oldGeneratingObject = mgr.GetIsGeneratingObject();
    mgr.SetIsGeneratingObject(true);
    auto p = mgr.GenerateObject(templateId);
    mgr.SetIsGeneratingObject(oldGeneratingObject);
    if (p.second != kInvalidUniqueId) {
      CEntity* newObj = mgr.ObjectById(p.second);
      CActor* newAct = TCastToPtr<CActor>(newObj).GetPtr();
      CScriptPickup* newPickup = TCastToPtr<CScriptPickup>(newObj).GetPtr();
      CActor* generatorAct = TCastToPtr<CActor>(generator).GetPtr();
      CWallCrawlerSwarm* swarmAct = TCastToPtr<CWallCrawlerSwarm>(generator).GetPtr();
      if (newAct && swarmAct)
        newAct->SetTranslation(swarmAct->GetLastKilledOffset() + x34_position);
      else if (newAct && generatorAct)
        newAct->SetTranslation(generatorAct->GetTranslation() + x34_position);
      if (newPickup)
        newPickup->SetGenerated();
      mgr.SendScriptMsg(newObj, GetUniqueId(), EScriptObjectMessage::Activate);
    }
  }
}

void CScriptPickupGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& stateMgr) {
  if (msg == EScriptObjectMessage::SetToZero && x30_24_active && x40_frequency != 100.f) {
    x44_delayTimer -= 1.f;
    if (x44_delayTimer < 0.000009f) {
      ResetDelayTimer();
    } else {
      std::vector<TUniqueId> generatorIds;
      GetGeneratorIds(stateMgr, sender, generatorIds);
      std::vector<std::pair<float, TEditorId>> pickupTemplates;
      float totalProb = GetPickupTemplates(stateMgr, pickupTemplates);
      if (!pickupTemplates.empty()) {
        float r = stateMgr.GetActiveRandom()->Range(0.f, totalProb);
        float f2 = 0.f;
        size_t count = 0;
        for (const auto id : pickupTemplates) {
          if (r >= f2 && r <= f2 + id.first)
            break;
          f2 += id.first;
          ++count;
        }
        if (count != pickupTemplates.size()) {
          TEditorId templateId = pickupTemplates[count].second;
          GeneratePickup(stateMgr, templateId,
                         generatorIds[stateMgr.GetActiveRandom()->Float() * generatorIds.size() * 0.99f]);
        }
      }
    }
  }

  CEntity::AcceptScriptMsg(msg, sender, stateMgr);
}

} // namespace urde
