#include "Runtime/Character/CModelData.hpp"

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
#include "Runtime/Graphics/CCubeRenderer.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CModelData");

CModelData::~CModelData() = default;

CModelData::CModelData() {}
CModelData CModelData::CModelDataNull() { return CModelData(); }

CModelData::CModelData(const CStaticRes& res) : x0_scale(res.GetScale()) {
  x1c_normalModel = g_SimplePool->GetObj({SBIG('CMDL'), res.GetId()});
  if (!x1c_normalModel)
    Log.report(logvisor::Fatal, FMT_STRING("unable to find CMDL {}"), res.GetId());
}

CModelData::CModelData(const CAnimRes& res) : x0_scale(res.GetScale()) {
  TToken<CCharacterFactory> factory = g_CharFactoryBuilder->GetFactory(res);
  x10_animData = factory->CreateCharacter(res.GetCharacterNodeId(), res.CanLoop(), factory, res.GetDefaultAnim());
}

SAdvancementDeltas CModelData::GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const {
  if (x10_animData)
    return x10_animData->GetAdvancementDeltas(a, b);
  else
    return {};
}

void CModelData::Render(const CStateManager& stateMgr, const zeus::CTransform& xf, const CActorLights* lights,
                        const CModelFlags& drawFlags) {
  Render(GetRenderingModel(stateMgr), xf, lights, drawFlags);
}

bool CModelData::IsLoaded(int shaderIdx) {
  if (x10_animData) {
    if (!x10_animData->xd8_modelData->GetModel()->IsLoaded(shaderIdx))
      return false;
    if (auto* model = x10_animData->xf4_xrayModel.get())
      if (!model->GetModel()->IsLoaded(shaderIdx))
        return false;
    if (auto* model = x10_animData->xf8_infraModel.get())
      if (!model->GetModel()->IsLoaded(shaderIdx))
        return false;
  }

  if (auto* model = x1c_normalModel.GetObj())
    if (!model->IsLoaded(shaderIdx))
      return false;
  if (auto* model = x2c_xrayModel.GetObj())
    if (!model->IsLoaded(shaderIdx))
      return false;
  if (auto* model = x3c_infraModel.GetObj())
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

TLockedToken<CModel>& CModelData::PickStaticModel(EWhichModel which) {
  switch (which) {
  case EWhichModel::XRay:
    if (x2c_xrayModel) {
      return x2c_xrayModel;
    }
    break;
  case EWhichModel::Thermal:
  case EWhichModel::ThermalHot:
    if (x3c_infraModel) {
      return x3c_infraModel;
    }
    break;
  default:
    break;
  }
  return x1c_normalModel;
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
      }
    }
  }
}

bool CModelData::IsDefinitelyOpaque(EWhichModel which) const {
  if (x10_animData) {
    CSkinnedModel& model = PickAnimatedModel(which);
    return model.GetModel()->IsOpaque();
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

void CModelData::Touch(EWhichModel which, int shaderIdx) {
  if (x10_animData)
    x10_animData->Touch(PickAnimatedModel(which), shaderIdx);
  else
    PickStaticModel(which)->Touch(shaderIdx);
}

void CModelData::Touch(const CStateManager& stateMgr, int shaderIdx) { Touch(GetRenderingModel(stateMgr), shaderIdx); }

void CModelData::RenderThermal(const zeus::CTransform& xf, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                               const CModelFlags& flags) {
  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  CGraphics::DisableAllLights();

  if (x10_animData) {
    CSkinnedModel& model = PickAnimatedModel(EWhichModel::ThermalHot);
    x10_animData->SetupRender(model, nullptr, nullptr);
    ThermalDraw(model, mulColor, addColor, flags);
  } else {
    auto& model = PickStaticModel(EWhichModel::ThermalHot);
    g_Renderer->DrawThermalModel(*model, mulColor, addColor, {}, {}, flags);
  }
}

void CModelData::RenderUnsortedParts(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                                     const CModelFlags& drawFlags) {
  if ((x14_25_sortThermal && which == EWhichModel::ThermalHot) || x10_animData || !x1c_normalModel ||
      drawFlags.x0_blendMode > 4) {
    x14_24_renderSorted = false;
    return;
  }

  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));

  if (lights != nullptr) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    g_Renderer->SetAmbientColor(x18_ambientColor);
  }

  PickStaticModel(which)->DrawUnsortedParts(drawFlags);
  g_Renderer->SetAmbientColor(zeus::skWhite);
  CGraphics::DisableAllLights();
  x14_24_renderSorted = true;
}

void CModelData::Render(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                        const CModelFlags& drawFlags) {
  if (x14_25_sortThermal && which == EWhichModel::ThermalHot) {
    zeus::CColor mul(drawFlags.x4_color.a(), drawFlags.x4_color.a());
    RenderThermal(xf, mul, {0.f, 0.f, 0.f, 0.25f}, drawFlags);
    return;
  }

  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));

  if (lights != nullptr) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    g_Renderer->SetAmbientColor(x18_ambientColor);
  }

  if (x10_animData) {
    x10_animData->Render(PickAnimatedModel(which), drawFlags, nullptr, nullptr);
  } else {
    // TODO supposed to be optional_object?
    if (x1c_normalModel) {
      auto& model = PickStaticModel(which);
      if (x14_24_renderSorted) {
        model->DrawSortedParts(drawFlags);
      } else {
        model->Draw(drawFlags);
      }
    }
  }

  g_Renderer->SetAmbientColor(zeus::skWhite);
  CGraphics::DisableAllLights();
  x14_24_renderSorted = false;
}

void CModelData::FlatDraw(EWhichModel which, const zeus::CTransform& xf, bool unsortedOnly, const CModelFlags& flags) {
  g_Renderer->SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  CGraphics::DisableAllLights();
  if (!x10_animData) {
    g_Renderer->DrawModelFlat(*PickStaticModel(which), flags, unsortedOnly, nullptr, nullptr);
  } else {
    auto model = PickAnimatedModel(which);
    x10_animData->SetupRender(model, nullptr, nullptr);
    model.DoDrawCallback([=](TConstVectorRef positions, TConstVectorRef normals) {
      auto m = model.GetModel();
      g_Renderer->DrawModelFlat(*m, flags, unsortedOnly, positions, normals);
    });
  }
}

void CModelData::MultiLightingDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                                   const zeus::CColor& alphaColor, const zeus::CColor& additiveColor) {
  CModel* model = nullptr;
  const auto callback = [&](auto positions, auto normals) {
    CGraphics::DisableAllLights();
    constexpr CModelFlags flags1{5, 0, 3, zeus::CColor{1.f, 0.f}};
    const CModelFlags flags2{5, 0, 1, alphaColor};
    const CModelFlags flags3{7, 0, 1, additiveColor};
    if (positions == nullptr) {
      model->Draw(flags1);
      if (lights != nullptr) {
        lights->ActivateLights();
      }
      model->Draw(flags2);
      model->Draw(flags3);
    } else {
      model->Draw(positions, normals, flags1);
      if (lights != nullptr) {
        lights->ActivateLights();
      }
      model->Draw(positions, normals, flags2);
      model->Draw(positions, normals, flags3);
    }
  };
  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  if (x10_animData) {
    auto& skinnedModel = PickAnimatedModel(which);
    x10_animData->SetupRender(skinnedModel, nullptr, nullptr);
    model = skinnedModel.GetModel().GetObj();
    skinnedModel.DoDrawCallback(callback);
  } else {
    model = PickStaticModel(which).GetObj();
    callback(nullptr, nullptr);
  }
}

void CModelData::MultiPassDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                               const CModelFlags* flags, u32 count) {
  CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_scale));
  if (lights == nullptr) {
    CGraphics::DisableAllLights();
    g_Renderer->SetAmbientColor(x18_ambientColor);
  } else {
    lights->ActivateLights();
  }
  if (x10_animData) {
    auto& skinnedModel = PickAnimatedModel(which);
    x10_animData->SetupRender(skinnedModel, nullptr, nullptr);
    auto& model = *skinnedModel.GetModel();
    skinnedModel.DoDrawCallback([&](auto positions, auto normals) {
      for (int i = 0; i < count; ++i) {
        model.Draw(positions, normals, flags[i]);
      }
    });
  } else {
    auto& model = *PickStaticModel(which);
    for (int i = 0; i < count; ++i) {
      model.Draw(flags[i]);
    }
  }
}

void CModelData::DisintegrateDraw(const CStateManager& mgr, const zeus::CTransform& xf, CTexture& tex,
                                  const zeus::CColor& addColor, float t) {
  DisintegrateDraw(GetRenderingModel(mgr), xf, tex, addColor, t);
}

void CModelData::DisintegrateDraw(EWhichModel which, const zeus::CTransform& xf, CTexture& tex,
                                  const zeus::CColor& addColor, float t) {
  zeus::CTransform scaledXf = xf * zeus::CTransform::Scale(x0_scale);
  CGraphics::SetModelMatrix(scaledXf);
  CGraphics::DisableAllLights();
  const auto aabb = GetBounds(scaledXf);
  if (x10_animData) {
    auto& model = PickAnimatedModel(which);
    x10_animData->SetupRender(model, nullptr, nullptr);
    model.DoDrawCallback([&](auto positions, auto normals) {
      g_Renderer->DrawModelDisintegrate(*model.GetModel(), tex, addColor, positions, normals, t);
    });
  } else {
    g_Renderer->DrawModelDisintegrate(*PickStaticModel(which), tex, addColor, nullptr, nullptr, t);
  }
}

void CModelData::ThermalDraw(CSkinnedModel& model, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                             const CModelFlags& flags) {
  model.DoDrawCallback([&](auto positions, auto normals) {
    g_Renderer->DrawThermalModel(*model.GetModel(), mulColor, addColor, positions, normals, flags);
  });
}

void CModelData::ThermalDraw(CSkinnedModel& model, TConstVectorRef positions, TConstVectorRef normals,
                             const zeus::CColor& mulColor, const zeus::CColor& addColor, const CModelFlags& flags) {
  g_Renderer->DrawThermalModel(*model.GetModel(), mulColor, addColor, positions, normals, flags);
}

} // namespace metaforce
