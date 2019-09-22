#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/Graphics/CLight.hpp"

namespace urde {
class CGameLight : public CActor {
  TUniqueId xe8_parentId;
  CLight xec_light;
  u32 x13c_lightId;
  u32 x140_priority;
  float x144_lifeTime;

public:
  CGameLight(TUniqueId, TAreaId, bool, std::string_view, const zeus::CTransform&, TUniqueId, const CLight&,
             u32 sourceId, u32, float);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void SetLightPriorityAndId();
  void SetLight(const CLight&);
  CLight GetLight() const;
  TUniqueId GetParentId() const { return xe8_parentId; }
};
} // namespace urde
