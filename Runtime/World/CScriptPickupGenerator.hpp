#pragma once

#include <string_view>

#include "Runtime/World/CEntity.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CScriptPickupGenerator : public CEntity {
  zeus::CVector3f x34_position;
  float x40_frequency;
  float x44_delayTimer = 0.f;
  void ResetDelayTimer();
  void GetGeneratorIds(CStateManager& mgr, TUniqueId sender, std::vector<TUniqueId>& idsOut) const;
  float GetPickupTemplates(CStateManager& mgr, std::vector<std::pair<float, TEditorId>>& idsOut) const;
  void GeneratePickup(CStateManager& mgr, TEditorId templateId, TUniqueId generatorId) const;

public:
  CScriptPickupGenerator(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&, float, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& stateMgr) override;
};
} // namespace urde
