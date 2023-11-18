#include "Runtime/Graphics/CSkinnedModel.hpp"

#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include <logvisor/logvisor.hpp>
#include <list>

namespace metaforce {
static logvisor::Module Log("metaforce::CSkinnedModel");

CSkinnedModel::CSkinnedModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules,
                             const TLockedToken<CCharLayoutInfo>& layoutInfo)
: x4_model(std::move(model))
, x10_skinRules(std::move(skinRules))
, x1c_layoutInfo(std::move(layoutInfo))
, m_workspace(*x10_skinRules) {
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

void CSkinnedModel::AllocateStorage() {
  if (x34_owned) {
    m_workspace.Reset(*x10_skinRules);
  }
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose, CVertexMorphEffect* morphEffect,
                              TConstVectorRef averagedNormals, SSkinningWorkspace* workspace) {
  if (workspace == nullptr) {
    if (x35_disableWorkspaces) {
      x10_skinRules->BuildAccumulatedTransforms(pose, *x1c_layoutInfo);
      return;
    }
    AllocateStorage();
    workspace = &m_workspace;
  } else {
    workspace->Reset(*x10_skinRules);
  }

  x10_skinRules->BuildAccumulatedTransforms(pose, *x1c_layoutInfo);
  x10_skinRules->BuildPoints(x4_model->GetPositions(), &workspace->m_vertexWorkspace);
  x10_skinRules->BuildNormals(x4_model->GetNormals(), &workspace->m_normalWorkspace);

  if (morphEffect) {
    morphEffect->MorphVertices(*workspace, averagedNormals, x10_skinRules, pose, x10_skinRules->GetVertexCount());
  }
  if (g_PointGenFunc != nullptr) {
    g_PointGenFunc(*workspace);
  }
}

void CSkinnedModel::Draw(TConstVectorRef verts, TConstVectorRef norms, const CModelFlags& drawFlags) {
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
  } else if (m_workspace.IsEmpty()) {
    x4_model->Draw(drawFlags);
  } else {
    x4_model->Draw(&m_workspace.m_vertexWorkspace, &m_workspace.m_normalWorkspace, drawFlags);
    // PostDrawFunc();
  }
}

void CSkinnedModel::DoDrawCallback(const FCustomDraw& func) const {
  if (x35_disableWorkspaces) {
    const auto mtx = CGraphics::g_GXModelMatrix;
    CGraphics::SetModelMatrix(mtx * x10_skinRules->x0_bones.front().x20_xf);
    func(x4_model->GetPositions(), x4_model->GetNormals());
    CGraphics::SetModelMatrix(mtx);
  } else if (m_workspace.IsEmpty()) {
    func(x4_model->GetPositions(), x4_model->GetNormals());
  } else {
    func(&m_workspace.m_vertexWorkspace, &m_workspace.m_normalWorkspace);
    // PostDrawFunc();
  }
}

void CSkinnedModel::CalculateDefault() { m_workspace.Clear(); }

SSkinningWorkspace CSkinnedModel::CloneWorkspace() { return m_workspace; }

CSkinnedModelWithAvgNormals::CSkinnedModelWithAvgNormals(IObjectStore& store, CAssetId model, CAssetId skinRules,
                                                         CAssetId layoutInfo)
: CSkinnedModel(store, model, skinRules, layoutInfo) {
  const auto vertexCount = GetSkinRules()->GetVertexCount();
  const auto& modelPositions = *GetModel()->GetPositions();

  x40_averagedNormals.resize(vertexCount);
  std::vector<std::pair<zeus::CVector3f, std::list<u32>>> vertMap;
  for (int vertIdx = 0; vertIdx < vertexCount; ++vertIdx) {
    const auto curPos = modelPositions[vertIdx];
    if (std::find_if(vertMap.cbegin(), vertMap.cend(), [=](const auto& pair) { return pair.first.isEqu(curPos); }) ==
        vertMap.cend()) {
      auto& [_, list] = vertMap.emplace_back(curPos, std::list<u32>{});
      for (int idx = vertIdx; idx < vertexCount; ++idx) {
        // Originally uses ==, but adjusted to match above
        if (modelPositions[idx].isEqu(curPos)) {
          list.emplace_back(idx);
        }
      }
    }
  }

  const auto& modelNormals = *GetModel()->GetNormals();
  for (const auto& [_, idxs] : vertMap) {
    zeus::CVector3f averagedNormal;
    for (const auto idx : idxs) {
      averagedNormal += modelNormals[idx];
    }
    averagedNormal.normalize();
    for (const auto idx : idxs) {
      x40_averagedNormals[idx] = averagedNormal;
    }
  }
}

FPointGenerator CSkinnedModel::g_PointGenFunc;

} // namespace metaforce
