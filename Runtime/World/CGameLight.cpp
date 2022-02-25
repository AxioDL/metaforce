#include "Runtime/World/CGameLight.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Graphics/CCubeRenderer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CGameLight::CGameLight(TUniqueId uid, TAreaId aid, bool active, std::string_view name, const zeus::CTransform& xf,
                       TUniqueId parentId, const CLight& light, u32 sourceId, u32 priority, float lifeTime)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf, CModelData::CModelDataNull(),
         CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
, xe8_parentId(parentId)
, xec_light(light)
, x13c_lightId(sourceId)
, x140_priority(priority)
, x144_lifeTime(lifeTime) {
  xec_light.GetRadius();
  xec_light.GetIntensity();
  SetLightPriorityAndId();
}

void CGameLight::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CGameLight::Think(float dt, CStateManager& mgr) {
  if (x144_lifeTime <= 0.f)
    return;
  x144_lifeTime -= dt;

  if (x144_lifeTime <= 0.f)
    mgr.FreeScriptObject(GetUniqueId());
}

void CGameLight::SetLightPriorityAndId() {
  xec_light.x3c_priority = x140_priority;
  xec_light.x40_lightId = x13c_lightId;
}

void CGameLight::SetLight(const CLight& light) {
  xec_light = light;
  xec_light.GetRadius();
  xec_light.GetIntensity();
  SetLightPriorityAndId();
}

CLight CGameLight::GetLight() const {
  CLight ret = xec_light;
  ret.SetPosition(x34_transform * xec_light.GetPosition());

  if (ret.GetType() != ELightType::Point)
    ret.SetDirection(x34_transform.rotate(xec_light.GetDirection()).normalized());

  return ret;
}

void CGameLight::DebugDraw() {
  if (!m_debugRes) {
    const auto* tok = (xec_light.GetType() == ELightType::Spot || xec_light.GetType() == ELightType::Directional)
                          ? g_ResFactory->GetResourceIdByName("CMDL_DebugLightCone")
                          : g_ResFactory->GetResourceIdByName("CMDL_DebugSphere");
    if (tok != nullptr && tok->type == FOURCC('CMDL')) {
      m_debugRes = CStaticRes(tok->id, zeus::skOne3f);
    }
  }

  if (m_debugRes && !m_debugModel) {
    m_debugModel = std::make_unique<CModelData>(*m_debugRes);
  }

  if (m_debugModel) {
    g_Renderer->SetGXRegister1Color(xec_light.GetColor());
    CModelFlags modelFlags;
    modelFlags.x0_blendMode = 5;
    modelFlags.x4_color = zeus::skWhite;
    modelFlags.x4_color.a() = 0.5f;
    m_debugModel->Render(CModelData::EWhichModel::Normal, zeus::CTransform::Translate(xec_light.GetPosition()), nullptr,
                         modelFlags);
    m_debugModel->Render(CModelData::EWhichModel::Normal, x34_transform, nullptr, modelFlags);
  }
}
} // namespace metaforce
