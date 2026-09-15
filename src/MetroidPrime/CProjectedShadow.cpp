#include "MetroidPrime/CProjectedShadow.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

#include <dolphin/gx.h>
#include <float.h>

struct SShadowDrawContext {
  const CSkinnedModel& model;
  bool drawAll;

  SShadowDrawContext(const CSkinnedModel& model, bool drawAll) : model(model), drawAll(drawAll) {}
};

CProjectedShadow::CProjectedShadow(const int w, const int h, const uchar persistent)
: x0_texture(kTF_I4, w, h, 1)
, x68_bounds(CAABox::MakeMaxInvertedBox())
, x80_enabled(false)
, x81_persistent(persistent)
, x84_scale(1.f)
, x88_translation(CVector3f::Zero())
, x94_zDistanceAdjust(0.f)
, x98_opacity(1.f)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x9c_nextShadow(nullptr)
#endif
{
}

CProjectedShadow::~CProjectedShadow() { x0_texture.ScheduleDeletion(); }

void CProjectedShadow::ModelDrawCallback(const float* positions, const float* normals,
                                         const SShadowDrawContext* context) {
  const CModel& model = **context->model.GetModel();
  const CCubeModel* cubeModel = model.GetCubeModel();
  model.UpdateLastFrame();
  cubeModel->DrawFlat(positions, normals, context->drawAll ? kSS_All : kSS_Unsorted);
}

void CProjectedShadow::ExpandBoundsForTexture() {
  const float texelScale = 1.f / (x0_texture.GetWidth() - 2);
  const CVector3f offset(texelScale * x68_bounds.GetWidth(), texelScale * x68_bounds.GetHeight(),
                         0.f);
  x68_bounds = CAABox(x68_bounds.GetMinPoint() - offset, x68_bounds.GetMaxPoint() + offset);
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void CProjectedShadow::RenderShadowBuffer(CStateManager& mgr, const CModelData& modelData,
                                          const CTransform4f& xf, int flags,
                                          const CVector3f& translation, float scale,
                                          float zDistanceAdjust) {
  const CModelData* model = &modelData;
  const CTransform4f* transform = &xf;
  RenderShadowBuffer(mgr, 1, &model, &transform, flags, translation, scale, zDistanceAdjust);
}

void CProjectedShadow::RenderShadowBuffer(CStateManager& mgr, int count,
                                          const CModelData* const* models,
                                          const CTransform4f* const* transforms, int flags,
                                          const CVector3f& translation, float scale,
                                          float zDistanceAdjust) {
  if (count < 1) {
    return;
  }

  x68_bounds = models[0]->GetBounds(*transforms[0]);
  for (int i = 1; i < count; ++i) {
    x68_bounds.Include(models[i]->GetBounds(*transforms[i]));
  }
#else
void CProjectedShadow::RenderShadowBuffer(CStateManager& mgr, const CModelData& modelData,
                                          const CTransform4f& xf, int flags,
                                          const CVector3f& translation, float scale,
                                          float zDistanceAdjust) {
  x68_bounds = modelData.GetBounds(xf);
#endif
  x84_scale = scale;
  x88_translation = translation;
  x94_zDistanceAdjust = zDistanceAdjust;
  x80_enabled = true;
  ExpandBoundsForTexture();

  const CTransform4f oldView = CGraphics::GetViewMatrix();
  const float oldNear = CGraphics::GetDepthNear();
  const float oldFar = CGraphics::GetDepthFar();
  const CGraphics::CProjectionState oldProjection = CGraphics::GetProjectionState();
  const CViewport oldViewport = CGraphics::GetViewport();
  const short width = x0_texture.GetWidth();
  const short height = x0_texture.GetHeight();
  const int renderWidth = width * 2;
  const int renderHeight = height * 2;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  const CVector3f center = (x68_bounds.GetMinPoint() + x68_bounds.GetMaxPoint()) * 0.5f;
#else
  const CVector3f center = x68_bounds.CenterPoint();
#endif
  const CTransform4f view = CTransform4f::FromColumns(
      CVector3f::Right(), CVector3f::Down(), CVector3f::Forward(),
      CVector3f(center.GetX(), center.GetY(), x68_bounds.GetMaxPoint().GetZ()));
  CGraphics::SetViewPointMatrix(view);
  CGraphics::SetDepthRange(0.f, 1.f);
  const float halfWidth = 0.5f * x68_bounds.GetWidth();
  const float halfHeight = 0.5f * x68_bounds.GetHeight();
  CGraphics::SetOrtho(-halfWidth, halfWidth, halfHeight, -halfHeight, 0.f,
                      FLT_EPSILON + x68_bounds.GetDepth());
  gpRender->SetViewport(0, CGraphics::GetRenderMode().efbHeight - renderHeight, renderWidth,
                        renderHeight);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGraphics::DisableAllLights();
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  for (int i = 0; i < count; ++i) {
    const CModelData& modelData = *models[i];
    const CTransform4f& xf = *transforms[i];
    CGraphics::SetModelMatrix(xf * CTransform4f::Scale(CVector3f(modelData.GetScale())));
    if (const CAnimData* animData = modelData.GetAnimationData()) {
      CSkinnedModel& model = modelData.PickAnimatedModel(CModelData::kWM_Normal);
      animData->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
      SShadowDrawContext context(model, flags == 0);
      model.Draw(reinterpret_cast< TDrawFunc >(ModelDrawCallback), &context);
    } else {
      const TLockedToken< CModel >& model = modelData.PickStaticModel(CModelData::kWM_Normal);
      model->UpdateLastFrame();
      model->GetCubeModel()->DrawFlat(nullptr, nullptr, flags == 0 ? kSS_All : kSS_Unsorted);
    }
  }
#else
  const CAnimData* animData = modelData.GetAnimationData();
  CSkinnedModel& model = modelData.PickAnimatedModel(CModelData::kWM_Normal);
  animData->SetupRender(model, rstl::optional_object< CVertexMorphEffect >(), nullptr);
  SShadowDrawContext context(model, flags == 0);
  CGraphics::SetModelMatrix(xf * CTransform4f::Scale(CVector3f(modelData.GetScale())));
  model.Draw(reinterpret_cast< TDrawFunc >(ModelDrawCallback), &context);

#endif

  bool useVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  CGX::SetZMode(true, GX_LEQUAL, true);
  GXSetTexCopySrc(0, 0, renderWidth, renderHeight);
  GXSetTexCopyDst(width, height, GX_CTF_R4, true);
  GXCopyTex(x0_texture.Lock(), true);
  x0_texture.UnLock();
  GXPixModeSync();
  CGraphics::SetUseVideoFilter(useVideoFilter);
  CGraphics::SetViewPointMatrix(oldView);
  CGraphics::SetProjectionState(oldProjection);
  gpRender->SetViewport(oldViewport.mLeft, oldViewport.mTop, oldViewport.mWidth,
                        oldViewport.mHeight);
  CGraphics::SetDepthRange(oldNear, oldFar);
  mgr.SetProjectedShadow(this);
}

CAABox ScaleAndTranslateBounds(const CAABox& bounds, const CVector3f& translation, float scale) {
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f padding = (extent * scale - extent) * 0.5f;
  return CAABox((bounds.GetMinPoint() - padding) + translation,
                (bounds.GetMaxPoint() + padding) + translation);
}

void CProjectedShadow::Render(const CStateManager& mgr) const {
  if (!x80_enabled) {
    return;
  }

  const CAABox bounds =
      ScaleAndTranslateBounds(x68_bounds, x88_translation, rstl::max_val(1.f, x84_scale));
  x0_texture.Load(GX_TEXMAP7, CTexture::kCM_Clamp);
  CGraphics::DisableAllLights();
  if (x81_persistent) {
    const uchar alpha = CCast::ToUint8(255.f * x98_opacity);
    const CColor color(alpha, alpha, alpha, alpha);
    const CLight light = CLight::BuildDirectional(CVector3f::Down(), color);
    CGraphics::LoadLight(kLight0, light);
    CGraphics::EnableLight(kLight0);
    const GXColor black = {0, 0, 0, 0};
    CGX::SetChanAmbColor(CGX::Channel0, black);
    const GXColor white = {255, 255, 255, 255};
    CGX::SetChanMatColor(CGX::Channel0, white);
    CGX::SetNumChans(0);
    CGX::FlushState();
    GXSetChanCtrl(GX_ALPHA0, true, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumChans(1);
  } else {
    CGX::SetNumChans(0);
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  if (x81_persistent) {
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR0A0);
  } else {
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR_NULL);
    CGX::SetTevKColor(GX_KCOLOR0, CColor::White().WithAlphaOf(x98_opacity).GetGXColor());
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  }
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetZMode(true, GX_EQUAL, false);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  static const Mtx textureFlip = {
      {1.f, 0.f, 0.f, 0.f}, {0.f, -1.f, 0.f, 1.f}, {0.f, 0.f, 0.f, 1.f}};
  CGX::LoadTexMtxImm(textureFlip, GX_PTTEXMTX0, GX_MTX3x4);
  CVector3f textureScale = bounds.GetMaxPoint() - bounds.GetMinPoint();
  textureScale.SetX(1.f / textureScale.GetX());
  textureScale.SetY(1.f / textureScale.GetY());
  textureScale.SetZ(1.f / textureScale.GetZ());
  const CTransform4f textureXf =
      CTransform4f::Scale(textureScale) * CTransform4f::Translate(-bounds.GetMinPoint());
  const CAABox queryBounds(bounds.GetMinPoint() - CVector3f(0.f, 0.f, x94_zDistanceAdjust),
                           bounds.GetMaxPoint());
  TEntityList nearList;
  mgr.BuildNearList(nearList, queryBounds, CMaterialFilter::skPassEverything, nullptr);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(*it));
    if (actor && actor->CanDrawStatic()) {
      const CModelData& modelData = *actor->GetModelData();
      const CTransform4f modelXf =
          actor->GetTransform() * CTransform4f::Scale(CVector3f(modelData.GetScale()));
      gpRender->SetModelMatrix(modelXf);
      const CTransform4f modelTextureXf = textureXf * modelXf;
      CGX::LoadTexMtxImm(modelTextureXf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
      const CModel& model = **modelData.PickStaticModel(CModelData::kWM_Normal);
      model.UpdateLastFrame();
      model.GetCubeModel()->DrawFlat(nullptr, nullptr, kSS_Unsorted);
    }
  }
  CGX::LoadTexMtxImm(textureXf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  gpRender->DrawXRayOutline(queryBounds, nullptr, nullptr);
  CGX::SetZMode(true, GX_LEQUAL, true);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  if (x81_persistent) {
    GXSetChanCtrl(GX_ALPHA0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
  }
}

void CProjectedShadow::Disable() { x80_enabled = false; }
