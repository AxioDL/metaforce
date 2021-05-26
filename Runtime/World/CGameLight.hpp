#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/Graphics/CLight.hpp"

namespace metaforce {
class CGameLight : public CActor {
  TUniqueId xe8_parentId;
  CLight xec_light;
  u32 x13c_lightId;
  u32 x140_priority;
  float x144_lifeTime;


  std::optional<CStaticRes> m_debugRes;
  std::unique_ptr<CModelData> m_debugModel;
public:
  DEFINE_ENTITY
  CGameLight(TUniqueId uid, TAreaId aid, bool active, std::string_view name, const zeus::CTransform& xf,
             TUniqueId parentId, const CLight& light, u32 sourceId, u32 priority, float lifeTime);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void SetLightPriorityAndId();
  void SetLight(const CLight& light);
  CLight GetLight() const;
  TUniqueId GetParentId() const { return xe8_parentId; }

  void DebugDraw();
};
} // namespace metaforce
