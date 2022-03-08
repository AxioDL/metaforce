#include "Runtime/Graphics/CSkinnedModel.hpp"

#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CSkinnedModel");

CSkinnedModel::CSkinnedModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules,
                             const TLockedToken<CCharLayoutInfo>& layoutInfo)
: x4_model(std::move(model)), x10_skinRules(std::move(skinRules)), x1c_layoutInfo(std::move(layoutInfo)) {
  if (!x4_model) {
    Log.report(logvisor::Fatal, FMT_STRING("bad model token provided to CSkinnedModel"));
  }
  if (!x10_skinRules) {
    Log.report(logvisor::Fatal, FMT_STRING("bad skin token provided to CSkinnedModel"));
  }
  if (!x1c_layoutInfo) {
    Log.report(logvisor::Fatal, FMT_STRING("bad character layout token provided to CSkinnedModel"));
  }
}

CSkinnedModel::CSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo)
: CSkinnedModel(store.GetObj(SObjectTag{FOURCC('CMDL'), model}), store.GetObj(SObjectTag{FOURCC('CSKR'), skinRules}),
                store.GetObj(SObjectTag{FOURCC('CINF'), layoutInfo})) {}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose, const std::optional<CVertexMorphEffect>& morphEffect,
                              const float* morphMagnitudes) {
  // TODO
  // if (morphEffect || g_PointGenFunc) {
  //   if (boo::ObjToken<boo::IGraphicsBufferD> vertBuf = m_modelInst->UpdateUniformData(drawFlags, nullptr, nullptr))
  //   {
  //     x10_skinRules->TransformVerticesCPU(m_vertWorkspace, pose, *x4_model);
  //     if (morphEffect)
  //       morphEffect->MorphVertices(m_vertWorkspace, morphMagnitudes, x10_skinRules, pose);
  //     if (g_PointGenFunc)
  //       g_PointGenFunc(g_PointGenCtx, m_vertWorkspace);
  //     x4_model->ApplyVerticesCPU(vertBuf, m_vertWorkspace);
  //     m_modifiedVBO = true;
  //   }
  // } else {
  //   if (boo::ObjToken<boo::IGraphicsBufferD> vertBuf =
  //           m_modelInst->UpdateUniformData(drawFlags, x10_skinRules.GetObj(), &pose)) {
  //     if (m_modifiedVBO) {
  //       x4_model->RestoreVerticesCPU(vertBuf);
  //       m_modifiedVBO = false;
  //     }
  //   }
  // }
}

void CSkinnedModel::Draw(TVectorRef verts, TVectorRef norms, const CModelFlags& drawFlags) {
  OPTICK_EVENT();
  x4_model->Draw(verts, norms, drawFlags);
  // PostDrawFunc();
}

void CSkinnedModel::Draw(const CModelFlags& drawFlags) {
  if (x35_disableWorkspaces) {
    const auto mtx = CGraphics::g_GXModelMatrix;
    CGraphics::SetModelMatrix(mtx * x10_skinRules->x0_bones.front().x20_xf);
    x4_model->Draw(drawFlags);
    CGraphics::SetModelMatrix(mtx);
  } else if (x24_vertWorkspace.empty()) {
    x4_model->Draw(drawFlags);
  } else {
    x4_model->Draw(&x24_vertWorkspace, &x2c_normalWorkspace, drawFlags);
    // PostDrawFunc();
  }
}

void CSkinnedModel::DoDrawCallback(const FCustomDraw& func) const {
  if (x35_disableWorkspaces) {
    const auto mtx = CGraphics::g_GXModelMatrix;
    CGraphics::SetModelMatrix(mtx * x10_skinRules->x0_bones.front().x20_xf);
    func(x4_model->GetPositions(), x4_model->GetNormals());
    CGraphics::SetModelMatrix(mtx);
  } else if (x24_vertWorkspace.empty()) {
    func(x4_model->GetPositions(), x4_model->GetNormals());
  } else {
    func(&x24_vertWorkspace, &x2c_normalWorkspace);
    // PostDrawFunc();
  }
}

CMorphableSkinnedModel::CMorphableSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules,
                                               CAssetId layoutInfo)
: CSkinnedModel(store, model, skinRules, layoutInfo) {}

CSkinnedModel::FPointGenerator CSkinnedModel::g_PointGenFunc = nullptr;
void* CSkinnedModel::g_PointGenCtx = nullptr;

} // namespace metaforce
