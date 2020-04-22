#include "Runtime/Character/CModelData.hpp"

#include "Editor/ProjectManager.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Character/CAdditiveAnimPlayback.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Character/CAssetFactory.hpp"
#include "Runtime/Character/CCharacterFactory.hpp"
#include "Runtime/Character/IAnimReader.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include <logvisor/logvisor.hpp>

namespace urde {
static logvisor::Module Log("urde::CModelData");

CModelData::~CModelData() = default;

CModelData::CModelData() {}
CModelData CModelData::CModelDataNull() { return CModelData(); }

CModelData::CModelData(const CStaticRes& res, int instCount) : x0_scale(res.GetScale()), m_drawInstCount(instCount) {
  x1c_normalModel = g_SimplePool->GetObj({SBIG('CMDL'), res.GetId()});
  if (!x1c_normalModel)
    Log.report(logvisor::Fatal, FMT_STRING("unable to find CMDL {}"), res.GetId());
  m_normalModelInst = x1c_normalModel->MakeNewInstance(0, instCount);
}

CModelData::CModelData(const CAnimRes& res, int instCount) : x0_scale(res.GetScale()), m_drawInstCount(instCount) {
  TToken<CCharacterFactory> factory = g_CharFactoryBuilder->GetFactory(res);
  x10_animData =
      factory->CreateCharacter(res.GetCharacterNodeId(), res.CanLoop(), factory, res.GetDefaultAnim(), instCount);
}

SAdvancementDeltas CModelData::GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const {
  if (x10_animData)
    return x10_animData->GetAdvancementDeltas(a, b);
  else
    return {};
}

void CModelData::Render(const CStateManager& stateMgr, const zeus::CTransform& xf, const CActorLights* lights,
                        const CModelFlags& drawFlags) const {
  Render(GetRenderingModel(stateMgr), xf, lights, drawFlags);
}

bool CModelData::IsLoaded(int shaderIdx) const {
  if (x10_animData) {
    if (!x10_animData->xd8_modelData->GetModel()->IsLoaded(shaderIdx))
      return false;
    if (const CSkinnedModel* model = x10_animData->xf4_xrayModel.get())
      if (!model->GetModel()->IsLoaded(shaderIdx))
        return false;
    if (const CSkinnedModel* model = x10_animData->xf8_infraModel.get())
      if (!model->GetModel()->IsLoaded(shaderIdx))
        return false;
  }

  if (const CModel* model = x1c_normalModel.GetObj())
    if (!model->IsLoaded(shaderIdx))
      return false;
  if (const CModel* model = x2c_xrayModel.GetObj())
    if (!model->IsLoaded(shaderIdx))
      return false;
  if (const CModel* model = x3c_infraModel.GetObj())
    if (!model->IsLoaded(shaderIdx))
      return false;

  return true;
}

u32 CModelData::GetNumMaterialSets() const {
  if (x10_animData)
    return x10_animData->GetModelData()->GetModel()->GetNumMaterialSets();

  if (x1c_normalModel)
    return x1c_normalModel->GetNumMaterialSets();

  return 1;
}

CModelData::EWhichModel CModelData::GetRenderingModel(const CStateManager& stateMgr) {
  switch (stateMgr.GetPlayerState()->GetActiveVisor(stateMgr)) {
  case CPlayerState::EPlayerVisor::XRay:
    return CModelData::EWhichModel::XRay;
  case CPlayerState::EPlayerVisor::Thermal:
    if (stateMgr.GetThermalDrawFlag() == EThermalDrawFlag::Cold)
      return CModelData::EWhichModel::Thermal;
    return CModelData::EWhichModel::ThermalHot;
  default:
    return CModelData::EWhichModel::Normal;
  }
}

CSkinnedModel& CModelData::PickAnimatedModel(EWhichModel which) const {
  CSkinnedModel* ret = nullptr;
  switch (which) {
  case EWhichModel::XRay:
    ret = x10_animData->xf4_xrayModel.get();
    break;
  case EWhichModel::Thermal:
  case EWhichModel::ThermalHot:
    ret = x10_animData->xf8_infraModel.get();
    break;
  default:
    break;
  }
  if (ret)
    return *ret;
  return *x10_animData->xd8_modelData.GetObj();
}

const std::unique_ptr<CBooModel>& CModelData::PickStaticModel(EWhichModel which) const {
  const std::unique_ptr<CBooModel>* ret = nullptr;
  switch (which) {
  case EWhichModel::XRay:
    ret = &m_xrayModelInst;
    break;
  case EWhichModel::Thermal:
  case EWhichModel::ThermalHot:
    ret = &m_infraModelInst;
    break;
  default:
    break;
  }
  if (ret && *ret)
    return *ret;
  return m_normalModelInst;
}

void CModelData::SetXRayModel(const std::pair<CAssetId, CAssetId>& modelSkin) {
  if (modelSkin.first.IsValid()) {
    if (g_ResFactory->GetResourceTypeById(modelSkin.first) == SBIG('CMDL')) {
      if (x10_animData && modelSkin.second.IsValid() &&
          g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR')) {
        x10_animData->SetXRayModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
                                   g_SimplePool->GetObj({SBIG('CSKR'), modelSkin.second}));
      } else {
        x2c_xrayModel = g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first});
        if (!x2c_xrayModel)
          Log.report(logvisor::Fatal, FMT_STRING("unable to find CMDL {}"), modelSkin.first);
        m_xrayModelInst = x2c_xrayModel->MakeNewInstance(0, m_drawInstCount);
      }
    }
  }
}

void CModelData::SetInfraModel(const std::pair<CAssetId, CAssetId>& modelSkin) {
  if (modelSkin.first.IsValid()) {
    if (g_ResFactory->GetResourceTypeById(modelSkin.first) == SBIG('CMDL')) {
      if (x10_animData && modelSkin.second.IsValid() &&
          g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR')) {
        x10_animData->SetInfraModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
                                    g_SimplePool->GetObj({SBIG('CSKR'), modelSkin.second}));
      } else {
        x3c_infraModel = g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first});
        if (!x3c_infraModel)
          Log.report(logvisor::Fatal, FMT_STRING("unable to find CMDL {}"), modelSkin.first);
        m_infraModelInst = x3c_infraModel->MakeNewInstance(0, m_drawInstCount);
      }
    }
  }
}

bool CModelData::IsDefinitelyOpaque(EWhichModel which) const {
  if (x10_animData) {
    CSkinnedModel& model = PickAnimatedModel(which);
    return model.GetModelInst()->IsOpaque();
  } else {
    const auto& model = PickStaticModel(which);
    return model->IsOpaque();
  }
}

bool CModelData::GetIsLoop() const {
  if (!x10_animData)
    return false;
  return x10_animData->GetIsLoop();
}

float CModelData::GetAnimationDuration(int idx) const {
  if (!x10_animData)
    return 0.f;
  return x10_animData->GetAnimationDuration(idx);
}

void CModelData::EnableLooping(bool enable) {
  if (!x10_animData)
    return;
  x10_animData->EnableLooping(enable);
}

void CModelData::AdvanceParticles(const zeus::CTransform& xf, float dt, CStateManager& stateMgr) {
  if (!x10_animData)
    return;
  x10_animData->AdvanceParticles(xf, dt, x0_scale, stateMgr);
}

zeus::CAABox CModelData::GetBounds() const {
  if (x10_animData) {
    return x10_animData->GetBoundingBox(zeus::CTransform::Scale(x0_scale));
  } else {
    const zeus::CAABox& aabb = x1c_normalModel->GetAABB();
    return zeus::CAABox(aabb.min * x0_scale, aabb.max * x0_scale);
  }
}

zeus::CAABox CModelData::GetBounds(const zeus::CTransform& xf) const {
  zeus::CTransform xf2 = xf * zeus::CTransform::Scale(x0_scale);
  if (x10_animData)
    return x10_animData->GetBoundingBox(xf2);
  else
    return x1c_normalModel->GetAABB().getTransformedAABox(xf2);
}

zeus::CTransform CModelData::GetScaledLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const {
  zeus::CTransform xf = GetLocatorTransformDynamic(name, time);
  xf.origin *= x0_scale;
  return xf;
}

zeus::CTransform CModelData::GetScaledLocatorTransform(std::string_view name) const {
  zeus::CTransform xf = GetLocatorTransform(name);
  xf.origin *= x0_scale;
  return xf;
}

zeus::CTransform CModelData::GetLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const {
  if (x10_animData)
    return x10_animData->GetLocatorTransform(name, time);
  else
    return {};
}

zeus::CTransform CModelData::GetLocatorTransform(std::string_view name) const {
  if (x10_animData)
    return x10_animData->GetLocatorTransform(name, nullptr);
  else
    return {};
}

SAdvancementDeltas CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool advTree) {
  if (x10_animData)
    return x10_animData->AdvanceIgnoreParticles(dt, rand, advTree);
  else
    return {};
}

SAdvancementDeltas CModelData::AdvanceAnimation(float dt, CStateManager& stateMgr, TAreaId aid, bool advTree) {
  if (x10_animData)
    return x10_animData->Advance(dt, x0_scale, stateMgr, aid, advTree);
  else
    return {};
}

bool CModelData::IsAnimating() const {
  if (!x10_animData)
    return false;
  return x10_animData->IsAnimating();
}

bool CModelData::IsInFrustum(const zeus::CTransform& xf, const zeus::CFrustum& frustum) const {
  if (!x10_animData && !x1c_normalModel)
    return true;
  return frustum.aabbFrustumTest(GetBounds(xf));
}

void CModelData::RenderParticles(const zeus::CFrustum& frustum) const {
  if (x10_animData)
    x10_animData->RenderAuxiliary(frustum);
}

void CModelData::Touch(EWhichModel which, int shaderIdx) const {
  if (x10_animData)
    x10_animData->Touch(PickAnimatedModel(which), shaderIdx);
  else
    PickStaticModel(which)->Touch(shaderIdx);
}

void CModelData::Touch(const CStateManager& stateMgr, int shaderIdx) const {
  Touch(const_cast<CModelData&>(*this).GetRenderingModel(stateMgr), shaderIdx);
}

void CModelData::RenderThermal(const zeus::CColor& mulColor, const zeus::CColor& addColor,
                               const CModelFlags& flags) const {
  CModelFlags drawFlags = flags;
  drawFlags.x4_color *= mulColor;
  drawFlags.addColor = addColor;
  drawFlags.m_extendedShader = EExtendedShader::Thermal;

  if (x10_animData) {
    CSkinnedModel& model = PickAnimatedModel(EWhichModel::ThermalHot);
    x10_animData->SetupRender(model, drawFlags, {}, nullptr);
    model.Draw(drawFlags);
  } else {
    const auto& model = PickStaticModel(EWhichModel::ThermalHot);
    model->Draw(drawFlags, nullptr, nullptr);
  }
}

void CModelData::RenderThermal(const zeus::CTransform& xf, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                               const CModelFlags& flags) const {
  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  CGraphics::DisableAllLights();
  RenderThermal(mulColor, addColor, flags);
}

void CModelData::RenderUnsortedParts(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                                     const CModelFlags& drawFlags) const {
  if ((x14_25_sortThermal && which == EWhichModel::ThermalHot) || x10_animData || !x1c_normalModel ||
      drawFlags.x0_blendMode > 4) {
    const_cast<CModelData*>(this)->x14_24_renderSorted = false;
    return;
  }

  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));

  const auto& model = PickStaticModel(which);
  if (lights) {
    lights->ActivateLights(*model);
  } else {
    std::vector<CLight> useLights;
    useLights.push_back(CLight::BuildLocalAmbient(zeus::skZero3f, x18_ambientColor));
    model->ActivateLights(useLights);
  }

  model->DrawNormal(drawFlags, nullptr, nullptr);
  // Set ambient to white
  CGraphics::DisableAllLights();
  const_cast<CModelData*>(this)->x14_24_renderSorted = true;
}

void CModelData::Render(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                        const CModelFlags& drawFlags) const {
  if (x14_25_sortThermal && which == EWhichModel::ThermalHot) {
    zeus::CColor mul(drawFlags.x4_color.a(), drawFlags.x4_color.a());
    RenderThermal(xf, mul, {0.f, 0.f, 0.f, 0.25f}, drawFlags);
  } else {
    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));

    if (x10_animData) {
      CSkinnedModel& model = PickAnimatedModel(which);
      if (lights) {
        lights->ActivateLights(*model.GetModelInst());
      } else {
        std::vector<CLight> useLights;
        useLights.push_back(CLight::BuildLocalAmbient(zeus::skZero3f, x18_ambientColor));
        model.GetModelInst()->ActivateLights(useLights);
      }

      x10_animData->Render(model, drawFlags, std::nullopt, nullptr);
    } else {
      const auto& model = PickStaticModel(which);
      if (lights) {
        lights->ActivateLights(*model);
      } else {
        std::vector<CLight> useLights;
        useLights.push_back(CLight::BuildLocalAmbient(zeus::skZero3f, x18_ambientColor));
        model->ActivateLights(useLights);
      }

      if (x14_24_renderSorted)
        model->DrawAlpha(drawFlags, nullptr, nullptr);
      else
        model->Draw(drawFlags, nullptr, nullptr);
    }

    // Set ambient to white
    CGraphics::DisableAllLights();
    const_cast<CModelData*>(this)->x14_24_renderSorted = false;
  }
}

void CModelData::InvSuitDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                             const zeus::CColor& alphaColor, const zeus::CColor& additiveColor) {
  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  if (x10_animData) {
    CSkinnedModel& model = PickAnimatedModel(which);
    model.GetModelInst()->DisableAllLights();
    CModelFlags flags = {};

    /* Z-prime */
    flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
    flags.x4_color = zeus::skWhite;
    x10_animData->Render(model, flags, std::nullopt, nullptr);

    /* Normal Blended */
    lights->ActivateLights(*model.GetModelInst());
    flags.m_extendedShader = EExtendedShader::ForcedAlpha;
    flags.x4_color = alphaColor;
    x10_animData->Render(model, flags, std::nullopt, nullptr);

    /* Selection Additive */
    flags.m_extendedShader = EExtendedShader::ForcedAdditive;
    flags.x4_color = additiveColor;
    x10_animData->Render(model, flags, std::nullopt, nullptr);
  } else {
    CBooModel& model = *PickStaticModel(which);
    model.DisableAllLights();
    CModelFlags flags = {};

    /* Z-prime */
    flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
    flags.x4_color = zeus::skWhite;
    model.Draw(flags, nullptr, nullptr);

    /* Normal Blended */
    lights->ActivateLights(model);
    flags.m_extendedShader = EExtendedShader::ForcedAlpha;
    flags.x4_color = alphaColor;
    model.Draw(flags, nullptr, nullptr);

    /* Selection Additive */
    flags.m_extendedShader = EExtendedShader::ForcedAdditive;
    flags.x4_color = additiveColor;
    model.Draw(flags, nullptr, nullptr);
  }
}

void CModelData::DisintegrateDraw(const CStateManager& mgr, const zeus::CTransform& xf, const CTexture& tex,
                                  const zeus::CColor& addColor, float t) {
  DisintegrateDraw(GetRenderingModel(mgr), xf, tex, addColor, t);
}

void CModelData::DisintegrateDraw(EWhichModel which, const zeus::CTransform& xf, const CTexture& tex,
                                  const zeus::CColor& addColor, float t) {
  zeus::CTransform scaledXf = xf * zeus::CTransform::Scale(x0_scale);
  CGraphics::SetModelMatrix(scaledXf);

  CBooModel::SetDisintegrateTexture(tex.GetBooTexture());
  CModelFlags flags(5, 0, 3, zeus::skWhite);
  flags.m_extendedShader = EExtendedShader::Disintegrate;
  flags.addColor = addColor;
  flags.addColor.a() = t; // Stash T value in here (shader does not care)

  if (x10_animData) {
    CSkinnedModel& sModel = PickAnimatedModel(which);
    x10_animData->Render(sModel, flags, std::nullopt, nullptr);
  } else {
    CBooModel& model = *PickStaticModel(which);
    model.Draw(flags, nullptr, nullptr);
  }
}

} // namespace urde
