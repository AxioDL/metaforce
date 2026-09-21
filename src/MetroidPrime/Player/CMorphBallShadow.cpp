#include "MetroidPrime/Player/CMorphBallShadow.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include <dolphin/gx.h>

#include <float.h>

CMorphBallShadow::CMorphBallShadow(int width, int height, const TToken< CTexture >& ballFade)
: x40_texture(kTF_I8, width, height, 1)
, xa8_ballFade(ballFade)
, xb0_width(width)
, xb4_height(height)
, xb8_shadowVolume(CAABox::MakeMaxInvertedBox())
, xd0_hasIds(false) {
  xa8_ballFade.Lock();
}

CMorphBallShadow::~CMorphBallShadow() { x40_texture.ScheduleDeletion(); }

void CMorphBallShadow::RenderIdBuffer(const CAABox& aabb, CStateManager& mgr, CPlayer& player) {
  xb8_shadowVolume = aabb;
  x0_actors.clear();
  x18_areas.clear();
  x30_worldModelBits = rstl::vector< uint >();

  CCubeRenderer* const renderer = gpRender;
  renderer->SetRequestRGBA6(true);
  if (!renderer->IsRGBA6Current()) {
    xd0_hasIds = false;
    return;
  }

  GatherAreas(mgr);
  const CTransform4f oldView = CGraphics::GetViewMatrix();
  const float oldNear = CGraphics::GetDepthNear();
  const float oldFar = CGraphics::GetDepthFar();
  const CGraphics::CProjectionState oldProjection = CGraphics::GetProjectionState();
  const CViewport oldViewport = CGraphics::GetViewport();
  const CVector3f center = aabb.CenterPoint();
  const CTransform4f view =
      CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Down(), CVector3f::Forward(),
                                CVector3f(center.GetX(), center.GetY(), aabb.GetMaxPoint().GetZ()));
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetDepthRange(0.f, 1.f);
  const float halfWidth = 0.5f * aabb.GetWidth();
  const float halfHeight = 0.5f * aabb.GetHeight();
  const float depth = aabb.GetDepth();
  CGraphics::SetOrtho(-halfWidth, halfWidth, halfHeight, -halfHeight, 0.f, FLT_EPSILON + depth);
  gpRender->SetViewport(0, CGraphics::GetRenderMode().efbHeight - xb4_height, xb0_width,
                        xb4_height);
  TEntityList nearList;
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::skPassEverything, &player);

  GXSetAlphaUpdate(true);
  GXSetDstAlpha(true, 0);
  GXSetColorUpdate(false);
  CGX::SetZMode(true, GX_ALWAYS, true);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  gpRender->BeginTriangleFan(4);
  gpRender->PrimColor(CColor::Red());
  const float clearDepth = 0.99f * depth;
  gpRender->PrimVertex(CVector3f(-halfWidth, clearDepth, -halfHeight));
  gpRender->PrimVertex(CVector3f(-halfWidth, clearDepth, halfHeight));
  gpRender->PrimVertex(CVector3f(halfWidth, clearDepth, halfHeight));
  gpRender->PrimVertex(CVector3f(halfWidth, clearDepth, -halfHeight));
  gpRender->EndPrimitive();
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetViewPointMatrix(view);
  CGX::SetZMode(true, GX_LEQUAL, true);
  CGraphics::DisableAllLights();
  CCubeModel::SetRenderModelBlack(true);

  int id = 1;
  for (AUTO(it, nearList.begin()); it != nearList.end() && id < 64; ++it) {
    CActor* actor = static_cast< CActor* >(const_cast< CEntity* >(mgr.GetObjectById(*it)));
    if (actor && actor->CanDrawStatic()) {
      x0_actors.push_back(actor);
      GXSetDstAlpha(true, id * 4);
      const CModelData& modelData = *actor->GetModelData();
      const CTransform4f modelXf =
          actor->GetTransform() * CTransform4f::Scale(CVector3f(modelData.GetScale()));
      gpRender->SetModelMatrix(modelXf);
      const CModelFlags flags(CModelFlags::kT_Opaque, 1.f);
      const CModel& model = **modelData.PickStaticModel(CModelData::kWM_Normal);
      model.DrawUnsortedParts(flags);
      ++id;
    }
  }
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  renderer->FindOverlappingWorldModels(x30_worldModelBits, aabb);
  id = renderer->DrawOverlappingWorldModelIDs(id, x30_worldModelBits, aabb, 0, 0);
  CCubeModel::SetRenderModelBlack(false);
  xd0_hasIds = id != 1;

  GXSetColorUpdate(true);
  GXSetDstAlpha(true, 0);
  bool useVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  GXSetTexCopySrc(0, 0, xb0_width, xb4_height);
  GXSetTexCopyDst(xb0_width, xb4_height, GX_CTF_A8, false);
  GXCopyTex(x40_texture.Lock(), true);
  x40_texture.UnLock();
  GXPixModeSync();
  CGraphics::SetUseVideoFilter(useVideoFilter);
  CGX::SetZMode(true, GX_LEQUAL, true);
  CGraphics::SetViewPointMatrix(oldView);
  CGraphics::SetProjectionState(oldProjection);
  gpRender->SetViewport(oldViewport.mLeft, oldViewport.mTop, oldViewport.mWidth,
                        oldViewport.mHeight);
  CGraphics::SetDepthRange(oldNear, oldFar);
}

void CMorphBallShadow::Render(CStateManager& mgr, float alpha) {
  CCubeRenderer& renderer = *gpRender;
  if (!xd0_hasIds || !AreasValid(mgr)) {
    return;
  }

  const bool hasFade = xa8_ballFade.IsLoaded();
  CGraphics::LoadDolphinSpareTexture(x40_texture.GetWidth(), x40_texture.GetHeight(), GX_TF_I8,
                                     const_cast< void* >(x40_texture.GetConstBitMapData(0)),
                                     CGraphics::kSpareBufferTexMapID);
  renderer.GetSphereRamp().Load(GX_TEXMAP1, CTexture::kCM_Clamp);
  if (hasFade) {
    TToken< CTexture >(xa8_ballFade)->Load(GX_TEXMAP2, CTexture::kCM_Clamp);
  }

  CGraphics::DisableAllLights();
  const CVector3f center = xb8_shadowVolume.GetCenterPoint();
  const CLight light = CLight::BuildDirectional(CVector3f::Down(), CColor(0.f, 0.f, 0.f, alpha));
  CGraphics::LoadLight(kLight0, light);
  const GXColor black = {0, 0, 0, 0};
  CGX::SetChanAmbColor(CGX::Channel0, GXColor(black));
  const GXColor white = {255, 255, 255, 255};
  CGX::SetChanMatColor(CGX::Channel0, GXColor(white));
  CGX::SetNumChans(0);
  CGX::FlushState();
  GXSetChanCtrl(GX_ALPHA0, true, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
  GXSetNumChans(1);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_EQ, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP7, GX_COLOR0A0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
  if (hasFade) {
    CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE2);
    CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);
    CGX::SetNumTevStages(3);
    CGX::SetNumTexGens(3);
  } else {
    CGX::SetNumTevStages(2);
    CGX::SetNumTexGens(2);
  }

  int id = 1;
  GXColor idColor = {0, 0, 0, 0};
  CGX::SetZMode(true, GX_EQUAL, false);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  static const Mtx textureFlip = {
      {1.f, 0.f, 0.f, 0.f}, {0.f, -1.f, 0.f, 1.f}, {0.f, 0.f, 0.f, 1.f}};
  CGX::LoadTexMtxImm(textureFlip, GX_PTTEXMTX0, GX_MTX3x4);
  CVector3f textureScale = xb8_shadowVolume.GetMaxPoint() - xb8_shadowVolume.GetMinPoint();
  textureScale.SetX(1.f / textureScale.GetX());
  textureScale.SetY(1.f / textureScale.GetY());
  textureScale.SetZ(1.f / textureScale.GetZ());
  const CTransform4f textureXf =
      CTransform4f::Scale(textureScale) * CTransform4f::Translate(-xb8_shadowVolume.GetMinPoint());
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  if (hasFade) {
    static const Mtx fadeMtx = {
        {0.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 1.f, -0.0625f}, {0.f, 0.f, 0.f, 1.f}};
    CGX::LoadTexMtxImm(fadeMtx, GX_PTTEXMTX1, GX_MTX3x4);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX1);
  }
  for (AUTO(it, x0_actors.begin()); it != x0_actors.end(); ++it) {
    idColor.a = id * 4;
    CActor* actor = *it;
    CGX::SetTevKColor(GX_KCOLOR0, idColor);
    const CModelData& modelData = *actor->GetModelData();
    const CTransform4f modelXf =
        actor->GetTransform() * CTransform4f::Scale(CVector3f(modelData.GetScale()));
    gpRender->SetModelMatrix(modelXf);
    const CTransform4f modelTextureXf = textureXf * modelXf;
    CGX::LoadTexMtxImm(modelTextureXf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
    const CModel& model = **modelData.PickStaticModel(CModelData::kWM_Normal);
    model.UpdateLastFrame();
    model.GetCubeModel()->DrawFlat(TModelPositions(), TModelNormals(), kSS_Unsorted);
    ++id;
  }
  CGX::LoadTexMtxImm(textureXf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  renderer.DrawOverlappingWorldModelShadows(id, x30_worldModelBits, xb8_shadowVolume, 0, 0);
  GXSetChanCtrl(GX_ALPHA0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  GXSetNumChans(0);
}

void CMorphBallShadow::GatherAreas(CStateManager& mgr) {
  x18_areas.clear();
  for (AUTO(it, mgr.GetWorld()->GetChainHead(CWorld::kC_Alive)); it != CWorld::skGlobalEnd; ++it) {
    if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
      x18_areas.push_back(TAreaId(it->GetAreaId()));
    }
  }
}

bool CMorphBallShadow::AreasValid(const CStateManager& mgr) const {
  AUTO(area, x18_areas.begin());
  for (AUTO(it, mgr.GetWorld()->GetChainHead(CWorld::kC_Alive)); it != CWorld::skGlobalEnd; ++it) {
    if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
      if (area == x18_areas.end()) {
        return false;
      }
      if (*area != it->GetAreaId()) {
        return false;
      }
      ++area;
    }
  }
  return true;
}
