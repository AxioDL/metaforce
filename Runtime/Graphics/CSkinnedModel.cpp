#include "Runtime/Graphics/CSkinnedModel.hpp"

#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include <logvisor/logvisor.hpp>

namespace urde {
static logvisor::Module Log("urde::CSkinnedModel");

CSkinnedModel::CSkinnedModel(TLockedToken<CModel> model, TLockedToken<CSkinRules> skinRules,
                             TLockedToken<CCharLayoutInfo> layoutInfo, int shaderIdx, int drawInsts)
: x4_model(std::move(model)), x10_skinRules(std::move(skinRules)), x1c_layoutInfo(std::move(layoutInfo)) {
  if (!x4_model) {
    Log.report(logvisor::Fatal, fmt("bad model token provided to CSkinnedModel"));
  }
  if (!x10_skinRules) {
    Log.report(logvisor::Fatal, fmt("bad skin token provided to CSkinnedModel"));
  }
  if (!x1c_layoutInfo) {
    Log.report(logvisor::Fatal, fmt("bad character layout token provided to CSkinnedModel"));
  }
  m_modelInst = x4_model->MakeNewInstance(shaderIdx, drawInsts);
}

CSkinnedModel::CSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo,
                             int shaderIdx, int drawInsts)
: CSkinnedModel(store.GetObj(SObjectTag{FOURCC('CMDL'), model}), store.GetObj(SObjectTag{FOURCC('CSKR'), skinRules}),
                store.GetObj(SObjectTag{FOURCC('CINF'), layoutInfo}), shaderIdx, drawInsts) {}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose, const CModelFlags& drawFlags,
                              const std::optional<CVertexMorphEffect>& morphEffect,
                              const float* morphMagnitudes) {
  if (morphEffect || g_PointGenFunc) {
    if (boo::ObjToken<boo::IGraphicsBufferD> vertBuf = m_modelInst->UpdateUniformData(drawFlags, nullptr, nullptr)) {
      x10_skinRules->TransformVerticesCPU(m_vertWorkspace, pose, *x4_model);
      if (morphEffect)
        morphEffect->MorphVertices(m_vertWorkspace, morphMagnitudes, x10_skinRules, pose);
      if (g_PointGenFunc)
        g_PointGenFunc(g_PointGenCtx, m_vertWorkspace);
      x4_model->ApplyVerticesCPU(vertBuf, m_vertWorkspace);
      m_modifiedVBO = true;
    }
  } else {
    if (boo::ObjToken<boo::IGraphicsBufferD> vertBuf =
            m_modelInst->UpdateUniformData(drawFlags, x10_skinRules.GetObj(), &pose)) {
      if (m_modifiedVBO) {
        x4_model->RestoreVerticesCPU(vertBuf);
        m_modifiedVBO = false;
      }
    }
  }
}

void CSkinnedModel::Draw(const CModelFlags& drawFlags) const {
  if (m_modelInst->TryLockTextures())
    m_modelInst->DrawSurfaces(drawFlags);
}

CMorphableSkinnedModel::CMorphableSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules,
                                               CAssetId layoutInfo, int shaderIdx, int drawInsts)
: CSkinnedModel(store, model, skinRules, layoutInfo, shaderIdx, drawInsts) {}

CSkinnedModel::FPointGenerator CSkinnedModel::g_PointGenFunc = nullptr;
void* CSkinnedModel::g_PointGenCtx = nullptr;

} // namespace urde
