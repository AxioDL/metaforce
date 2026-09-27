#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

struct SThermalDrawContext {
  SThermalDrawContext(CSkinnedModel& model, const CColor& mulColor, const CColor& addColor,
                      const CModelFlags& flags)
  : model(model), mulColor(mulColor), addColor(addColor), flags(flags) {}
  CSkinnedModel& model;
  CColor mulColor;
  CColor addColor;
  const CModelFlags& flags;
};
struct SOneTextureDrawContext {
  SOneTextureDrawContext(CSkinnedModel& model, const CTexture& texture, const CColor& color,
                         float t)
  : model(model), texture(texture), color(color), t(t) {}
  CSkinnedModel& model;
  const CTexture& texture;
  CColor color;
  float t;
};
struct SFlatDrawContext {
  SFlatDrawContext(CSkinnedModel& model, bool unsortedOnly, const CModelFlags& flags)
  : model(model), unsortedOnly(unsortedOnly), flags(flags) {}
  CSkinnedModel& model;
  bool unsortedOnly;
  const CModelFlags& flags;
};
struct SMultiLightingDrawContext {
  SMultiLightingDrawContext(const CModel& model, const CActorLights* lights,
                            const CColor& alphaColor, const CColor& additiveColor)
  : model(model), lights(lights), alphaColor(alphaColor), additiveColor(additiveColor) {}
  const CModel& model;
  const CActorLights* lights;
  const CColor& alphaColor;
  const CColor& additiveColor;
};
struct SMultipassDrawContext {
  SMultipassDrawContext(CSkinnedModel& model, const CModelFlags* flags, int count)
  : model(model), flags(flags), count(count) {}
  CSkinnedModel& model;
  const CModelFlags* flags;
  int count;
};

static const CAdvancementDeltas skNullAdvance(CVector3f::Zero(), CQuaternion::NoRotation());

CModelData::CModelData(const CStaticRes& res)
: mScale(res.GetScale())
, mRenderSorted(false)
, mSortThermal(false)
, mAmbientColor(CColor::White())
, mNormalModel(gpSimplePool->GetObj(SObjectTag('CMDL', res.GetId()))) {}

CModelData::CModelData()
: mScale(1.f, 1.f, 1.f)
, mRenderSorted(false)
, mSortThermal(false)
, mAmbientColor(CColor::White()) {}

CModelData::CModelData(const CAnimRes& res)
: mScale(res.GetScale())
, mRenderSorted(false)
, mSortThermal(false)
, mAmbientColor(CColor::White()) {
  TLockedToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(res);
  mAnimData =
      factory
          ->CreateCharacter(res.GetCharacterNodeId(), res.CanLoop(), factory, res.GetDefaultAnim())
          .release();
}

CModelData::~CModelData() {}

void CModelData::Render(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                        const CModelFlags& flags) const {
  if (mSortThermal && which == kWM_ThermalHot) {
    const float alpha = flags.GetColorRef().GetAlpha();
    const CColor mulColor(alpha, alpha, alpha, alpha);
    RenderThermal(xf, mulColor, CColor(uchar(0), uchar(0), uchar(0), uchar(0x40)), flags);
    return;
  }
  CTransform4f scaledXf = xf;
  scaledXf *= CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
  gpRender->SetModelMatrix(scaledXf);
  if (lights != nullptr) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }
  if (HasAnimation()) {
    mAnimData->Render(PickAnimatedModel(which), flags,
                        rstl::optional_object< CVertexMorphEffect >(), nullptr);
  } else if (mNormalModel) {
    const CModel& model = **PickStaticModel(which);
    if (mRenderSorted)
      model.DrawSortedParts(flags);
    else
      model.Draw(flags);
  }
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  mRenderSorted = false;
}

void CModelData::RenderUnsortedParts(EWhichModel which, const CTransform4f& xf,
                                     const CActorLights* lights, const CModelFlags& flags) const {
  if ((mSortThermal && which == kWM_ThermalHot) || HasAnimation() || !mNormalModel ||
      flags.GetTrans() > CModelFlags::kT_Four) {
    mRenderSorted = false;
    return;
  }
  CTransform4f scaledXf =
      xf * CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
  gpRender->SetModelMatrix(scaledXf);
  if (lights != nullptr) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }
  PickStaticModel(which)->DrawUnsortedParts(flags);
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  mRenderSorted = true;
}

void CModelData::ThermalDrawCallback(TModelPositions positions, TModelNormals normals,
                                     const SThermalDrawContext* ctx) {
  gpRender->DrawThermalModel(**ctx->model.GetModel(), ctx->mulColor, ctx->addColor, positions,
                             normals, ctx->flags);
}

void CModelData::DisintegrateDrawCallback(TModelPositions positions, TModelNormals normals,
                                          const SOneTextureDrawContext* ctx) {
  gpRender->DrawModelDisintegrate(**ctx->model.GetModel(), ctx->texture, ctx->color, positions,
                                  normals, ctx->t);
}

void CModelData::FlatDrawCallback(TModelPositions positions, TModelNormals normals,
                                  const SFlatDrawContext* ctx) {
  gpRender->DrawModelFlat(**ctx->model.GetModel(), ctx->flags, ctx->unsortedOnly, positions,
                          normals);
}

void CModelData::MultiLightingDrawCallback(TModelPositions positions, TModelNormals normals,
                                           SMultiLightingDrawContext* ctx) {
  CGraphics::DisableAllLights();
  CModelFlags depthFlags = CModelFlags::AlphaBlended(0.f);
  CModelFlags alphaFlags =
      CModelFlags::AlphaBlended(ctx->alphaColor).DepthCompareUpdate(true, false);
  CModelFlags additiveFlags =
      CModelFlags::Additive(ctx->additiveColor).DepthCompareUpdate(true, false);
#if defined(TARGET_PC)
  if (!positions.empty()) {
#else
  if (positions != nullptr) {
#endif
    ctx->model.Draw(positions, normals, depthFlags);
    if (ctx->lights != nullptr)
      ctx->lights->ActivateLights();
    ctx->model.Draw(positions, normals, alphaFlags);
    ctx->model.Draw(positions, normals, additiveFlags);
  } else {
    ctx->model.Draw(depthFlags);
    if (ctx->lights != nullptr)
      ctx->lights->ActivateLights();
    ctx->model.Draw(alphaFlags);
    ctx->model.Draw(additiveFlags);
  }
}

void CModelData::MultipassDrawCallback(TModelPositions positions, TModelNormals normals,
                                       const SMultipassDrawContext* ctx) {
  const CModel& model = **ctx->model.GetModel();
  for (int i = 0; i < ctx->count; ++i)
    model.Draw(positions, normals, ctx->flags[i]);
}

void CModelData::ThermalDraw(CSkinnedModel& model, const CColor& mulColor, const CColor& addColor,
                             const CModelFlags& flags) {
  SThermalDrawContext ctx(model, mulColor, addColor, flags);
  model.Draw(ThermalDrawCallback, &ctx);
}

void CModelData::ThermalDraw(CSkinnedModel& model, const float* positions, const float* normals,
                             const CColor& mulColor, const CColor& addColor,
                             const CModelFlags& flags) {
#if defined(TARGET_PC)
  gpRender->DrawThermalModel(**model.GetModel(), mulColor, addColor,
                             model.GetPositionView(positions), model.GetNormalView(normals), flags);
#else
  gpRender->DrawThermalModel(**model.GetModel(), mulColor, addColor, positions, normals, flags);
#endif
}

void CModelData::RenderThermal(const CTransform4f& xf, const CColor& mulColor,
                               const CColor& addColor, const CModelFlags& flags) const {
  CTransform4f scaledXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(scaledXf);
  CGraphics::DisableAllLights();
  if (HasAnimation()) {
    CSkinnedModel& model = PickAnimatedModel(kWM_ThermalHot);
    mAnimData->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
    ThermalDraw(model, mulColor, addColor, flags);
  } else {
    gpRender->DrawThermalModel(**PickStaticModel(kWM_ThermalHot), mulColor, addColor,
                               TModelPositions(), TModelNormals(), flags);
  }
}

void CModelData::DisintegrateDraw(EWhichModel which, const CTransform4f& xf,
                                  const CTexture& texture, const CColor& color, float t) const {
  CTransform4f scaledXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(scaledXf);
  CGraphics::DisableAllLights();
  CAABox bounds = GetBounds(scaledXf);
  if (HasAnimation()) {
    CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
    SOneTextureDrawContext ctx(model, texture, color, t);
    model.Draw(DisintegrateDrawCallback, &ctx);
  } else {
    gpRender->DrawModelDisintegrate(**PickStaticModel(which), texture, color, TModelPositions(),
                                    TModelNormals(), t);
  }
}

void CModelData::DisintegrateDraw(const CStateManager& mgr, const CTransform4f& xf,
                                  const CTexture& texture, const CColor& color, float t) const {
  DisintegrateDraw(GetRenderingModel(mgr), xf, texture, color, t);
}

void CModelData::FlatDraw(EWhichModel which, const CTransform4f& xf, bool unsortedOnly,
                          const CModelFlags& flags) const {
  CTransform4f scaledXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(scaledXf);
  CGraphics::DisableAllLights();
  if (HasAnimation()) {
    CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
    SFlatDrawContext ctx(model, unsortedOnly, flags);
    model.Draw(FlatDrawCallback, &ctx);
  } else {
    gpRender->DrawModelFlat(**PickStaticModel(which), flags, unsortedOnly, TModelPositions(),
                            TModelNormals());
  }
}

void CModelData::MultiLightingDraw(EWhichModel which, const CTransform4f& xf,
                                   const CActorLights* lights, const CColor& mulColor,
                                   const CColor& addColor) const {
  CTransform4f scaledXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(scaledXf);
  if (HasAnimation()) {
    CSkinnedModel& model = PickAnimatedModel(which);
    GetAnimationData()->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
    SMultiLightingDrawContext ctx(**model.GetModel(), lights, mulColor, addColor);
    model.Draw(MultiLightingDrawCallback, &ctx);
  } else {
    SMultiLightingDrawContext ctx(**PickStaticModel(which), lights, mulColor, addColor);
    MultiLightingDrawCallback(TModelPositions(), TModelNormals(), &ctx);
  }
}

void CModelData::MultipassDraw(EWhichModel which, const CTransform4f& xf,
                               const CActorLights* lights, const CModelFlags* flags,
                               int count) const {
  CTransform4f scaledXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(scaledXf);
  if (lights != nullptr) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }
  if (HasAnimation()) {
    CSkinnedModel& model = PickAnimatedModel(which);
    GetAnimationData()->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
    SMultipassDrawContext ctx(model, flags, count);
    model.Draw(MultipassDrawCallback, &ctx);
  } else {
    const CModel& model = **PickStaticModel(which);
    for (int i = 0; i < count; ++i)
      model.Draw(flags[i]);
  }
}

void CModelData::Touch(const CStateManager& mgr, int shaderIdx) const {
  Touch(GetRenderingModel(mgr), shaderIdx);
}

void CModelData::Touch(EWhichModel which, int shaderIdx) const {
  if (HasAnimation())
    mAnimData->Touch(PickAnimatedModel(which), shaderIdx);
  else
    PickStaticModel(which)->Touch(shaderIdx);
}

void CModelData::RenderParticles(const CFrustumPlanes& planes) const {
  if (HasAnimation())
    mAnimData->RenderAuxiliary(planes);
}

bool CModelData::IsInFrustum(const CTransform4f& xf, const CFrustumPlanes& planes) const {
  if (IsNull())
    return true;
  return planes.BoxInFrustumPlanes(GetBounds(xf));
}

bool CModelData::IsAnimating() const { return HasAnimation() && mAnimData->IsAnimating(); }

CAdvancementDeltas CModelData::AdvanceAnimation(float dt, CStateManager& mgr, TAreaId aid,
                                                bool advTree) {
  if (!HasAnimation())
    return skNullAdvance;
  return mAnimData->Advance(dt, ScaleCopy(), mgr, aid, advTree);
}

CAdvancementDeltas CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand,
                                                               bool advTree) {
  if (!HasAnimation())
    return skNullAdvance;
  return mAnimData->AdvanceIgnoreParticles(dt, rand, advTree);
}

CTransform4f CModelData::GetLocatorTransform(const rstl::string& name) const {
  if (!HasAnimation())
    return CTransform4f::Identity();
  return mAnimData->GetLocatorTransform(name, nullptr);
}

CTransform4f CModelData::GetLocatorTransformDynamic(const rstl::string& name,
                                                    const CCharAnimTime* time) const {
  if (!HasAnimation())
    return CTransform4f::Identity();
  return mAnimData->GetLocatorTransform(name, time);
}

CTransform4f CModelData::GetScaledLocatorTransform(const rstl::string& name) const {
  CTransform4f xf = GetLocatorTransform(name);
  const CVector3f pos = xf.GetTranslation();
  xf.SetTranslation(CVector3f(mScale.GetX() * pos.GetX(), mScale.GetY() * pos.GetY(),
                              mScale.GetZ() * pos.GetZ()));
  return xf;
}

CTransform4f CModelData::GetScaledLocatorTransformDynamic(const rstl::string& name,
                                                          const CCharAnimTime* time) const {
  CTransform4f xf = GetLocatorTransformDynamic(name, time);
  const CVector3f pos = xf.GetTranslation();
  xf.SetTranslation(CVector3f(mScale.GetX() * pos.GetX(), mScale.GetY() * pos.GetY(),
                              mScale.GetZ() * pos.GetZ()));
  return xf;
}

CAABox CModelData::GetBounds(const CTransform4f& xf) const {
  CTransform4f scaledXf =
      xf * CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
  if (HasAnimation())
    return mAnimData->GetBoundingBox(scaledXf);
  if (!mXrayModel && !mInfraModel)
    return (*mNormalModel)->GetBoundingBox().GetTransformedAABox(scaledXf);
  CAABox bounds = (*mNormalModel)->GetBoundingBox();
  if (mInfraModel) {
    bounds.Include((*mInfraModel)->GetBoundingBox());
  }
  if (mXrayModel) {
    bounds.Include((*mXrayModel)->GetBoundingBox());
  }
  return bounds.GetTransformedAABox(scaledXf);
}

CAABox CModelData::GetBounds() const {
  if (HasAnimation())
    return mAnimData->GetBoundingBox(
        CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ()));
  if (!mXrayModel && !mInfraModel) {
    const CAABox& bounds = (*mNormalModel)->GetBoundingBox();
    return CAABox(CVector3f(bounds.GetMinPoint().GetX() * mScale.GetX(),
                            bounds.GetMinPoint().GetY() * mScale.GetY(),
                            bounds.GetMinPoint().GetZ() * mScale.GetZ()),
                  CVector3f(bounds.GetMaxPoint().GetX() * mScale.GetX(),
                            bounds.GetMaxPoint().GetY() * mScale.GetY(),
                            bounds.GetMaxPoint().GetZ() * mScale.GetZ()));
  }
  CAABox bounds = (*mNormalModel)->GetBoundingBox();
  if (mInfraModel) {
    bounds.Include((*mInfraModel)->GetBoundingBox());
  }
  if (mXrayModel) {
    bounds.Include((*mXrayModel)->GetBoundingBox());
  }
  return CAABox(CVector3f(bounds.GetMinPoint().GetX() * mScale.GetX(),
                          bounds.GetMinPoint().GetY() * mScale.GetY(),
                          bounds.GetMinPoint().GetZ() * mScale.GetZ()),
                CVector3f(bounds.GetMaxPoint().GetX() * mScale.GetX(),
                          bounds.GetMaxPoint().GetY() * mScale.GetY(),
                          bounds.GetMaxPoint().GetZ() * mScale.GetZ()));
}

void CModelData::AdvanceParticles(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (HasAnimation())
    mAnimData->AdvanceParticles(xf, dt, mScale, mgr);
}

void CModelData::EnableLooping(bool enable) {
  if (!HasAnimation())
    return;
  mAnimData->EnableLooping(enable);
}

float CModelData::GetAnimationDuration(int anim) const {
  if (mAnimData.null())
    return 0.f;
  return mAnimData->GetAnimationDuration(anim);
}

bool CModelData::GetIsLoop() const {
  if (mAnimData.null())
    return false;
  return mAnimData->GetIsLoop();
}

bool CModelData::IsDefinitelyOpaque(EWhichModel which) const {
  if (HasAnimation())
    return PickAnimatedModel(which).GetModel()->IsDefinitelyOpaque();
  if (mNormalModel)
    return PickStaticModel(which)->IsDefinitelyOpaque();
  return false;
}

void CModelData::SetInfraModel(const rstl::pair< CAssetId, CAssetId >& assets) {
  if (assets.first != 0) {
    if (HasAnimation() && assets.second != 0) {
      mAnimData->SetInfraModel(
          TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first))),
          TToken< CSkinRules >(gpSimplePool->GetObj(SObjectTag('CSKR', assets.second))));
    } else {
      mInfraModel =
          TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)));
    }
  }
}

void CModelData::SetXRayModel(const rstl::pair< CAssetId, CAssetId >& assets) {
  if (assets.first == 0 || gpResourceFactory->GetResourceTypeById(assets.first) != 'CMDL')
    return;
  if (HasAnimation() && assets.second != 0 &&
      gpResourceFactory->GetResourceTypeById(assets.second) == 'CSKR') {
    mAnimData->SetXRayModel(
        TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first))),
        TToken< CSkinRules >(gpSimplePool->GetObj(SObjectTag('CSKR', assets.second))));
  } else {
    mXrayModel = TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)));
  }
}

const TLockedToken< CModel >& CModelData::PickStaticModel(EWhichModel which) const {
  switch (which) {
  case kWM_Thermal:
  case kWM_ThermalHot:
    if (mInfraModel)
      return *mInfraModel;
    break;
  case kWM_XRay:
    if (mXrayModel)
      return *mXrayModel;
    break;
  default:
    break;
  }
  return *mNormalModel;
}

CSkinnedModel& CModelData::PickAnimatedModel(EWhichModel which) const {
  CSkinnedModel* model = nullptr;
  switch (which) {
  case kWM_Thermal:
  case kWM_ThermalHot:
    model = mAnimData->GetInfraModel();
    break;
  case kWM_XRay:
    model = mAnimData->GetXRayModel();
    break;
  default:
    break;
  }
  if (model != nullptr)
    return *model;
  return **mAnimData->GetModelData();
}

CModelData::EWhichModel CModelData::GetRenderingModel(const CStateManager& mgr) {
  switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_XRay:
    return kWM_XRay;
  case CPlayerState::kPV_Thermal:
    if (mgr.GetThermalDrawFlag() == kTD_Cold)
      return kWM_Thermal;
    return kWM_ThermalHot;
  default:
    return kWM_Normal;
  }
}

bool CModelData::HasModel(EWhichModel which) const {
  if (HasAnimation()) {
    switch (which) {
    case kWM_Normal:
      return true;
    case kWM_XRay:
      return mAnimData->GetXRayModel() != nullptr;
    case kWM_Thermal:
    case kWM_ThermalHot:
      return mAnimData->GetInfraModel() != nullptr;
    default:
      return false;
    }
  }
  switch (which) {
  case kWM_Normal:
    return mNormalModel;
  case kWM_XRay:
    return mXrayModel;
  case kWM_Thermal:
  case kWM_ThermalHot:
    return mInfraModel;
  default:
    return false;
  }
}

void CModelData::Render(const CStateManager& mgr, const CTransform4f& xf,
                        const CActorLights* lights, const CModelFlags& flags) const {
  Render(GetRenderingModel(mgr), xf, lights, flags);
}

bool CModelData::IsLoaded(int shaderIdx) const {
  if (HasAnimation()) {
    const CAnimData& animData = *mAnimData;
    if (!animData.GetModelData()->GetModel()->IsLoaded(shaderIdx))
      return false;
    CSkinnedModel* xrayModel = animData.GetXRayModel();
    CSkinnedModel* infraModel = animData.GetInfraModel();
    if (xrayModel != nullptr && !xrayModel->GetModel()->IsLoaded(shaderIdx))
      return false;
    if (infraModel != nullptr && !infraModel->GetModel()->IsLoaded(shaderIdx))
      return false;
  }
  if (mNormalModel && !(*mNormalModel)->IsLoaded(shaderIdx))
    return false;
  if (mXrayModel && !(*mXrayModel)->IsLoaded(shaderIdx))
    return false;
  if (mInfraModel && !(*mInfraModel)->IsLoaded(shaderIdx))
    return false;
  return true;
}

int CModelData::GetNumMaterialSets() const {
  if (HasAnimation())
    return mAnimData->GetModelData()->GetModel()->GetNumMaterialSets();
  if (mNormalModel)
    return (*mNormalModel)->GetNumMaterialSets();
  return 1;
}
