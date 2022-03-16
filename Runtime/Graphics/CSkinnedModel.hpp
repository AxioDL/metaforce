#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CModel.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCharLayoutInfo;
class CModel;
class CPoseAsTransforms;
class CVertexMorphEffect;
class IObjectStore;

// Originally vert + normal workspaces were allocated together, but here separated for ease of use
struct SSkinningWorkspace {
  std::vector<zeus::CVector3f> m_vertexWorkspace;
  std::vector<zeus::CVector3f> m_normalWorkspace;

  SSkinningWorkspace(const CSkinRules& rules) { Reset(rules); }
  void Reset(const CSkinRules& rules) {
    m_vertexWorkspace.clear();
    m_normalWorkspace.clear();
    m_vertexWorkspace.reserve(rules.GetVertexCount());
    m_normalWorkspace.reserve(rules.GetNormalCount());
  }
  [[nodiscard]] bool IsEmpty() const { return m_vertexWorkspace.empty() || m_normalWorkspace.empty(); }
};

// Lambda instead of userdata pointer
using FCustomDraw = std::function<void(TConstVectorRef positions, TConstVectorRef normals)>;
using FPointGenerator = std::function<void(const SSkinningWorkspace& workspace)>;

class CSkinnedModel {
  TLockedToken<CModel> x4_model;
  TLockedToken<CSkinRules> x10_skinRules;
  TLockedToken<CCharLayoutInfo> x1c_layoutInfo;
  // rstl::auto_ptr<float[]> x24_vertWorkspace;
  // rstl::auto_ptr<float[]> x2c_normalWorkspace;
  SSkinningWorkspace m_workspace;
  bool x34_owned = true;
  bool x35_disableWorkspaces = false;

public:
  enum class EDataOwnership { Unowned, Owned };
  CSkinnedModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules,
                const TLockedToken<CCharLayoutInfo>& layoutInfo /*, EDataOwnership ownership*/);
  CSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo);
  virtual ~CSkinnedModel() = default;

  TLockedToken<CModel>& GetModel() { return x4_model; }
  const TLockedToken<CModel>& GetModel() const { return x4_model; }
  const TLockedToken<CSkinRules>& GetSkinRules() const { return x10_skinRules; }
  void SetLayoutInfo(const TLockedToken<CCharLayoutInfo>& inf) { x1c_layoutInfo = inf; }
  const TLockedToken<CCharLayoutInfo>& GetLayoutInfo() const { return x1c_layoutInfo; }

  void AllocateStorage();
  // Metaforce addition: Originally morphEffect is rstl::optional_object<CVertexMorphEffect>*
  // This prevents constructing it as a reference to the held pointer in CPatterned, thus in
  // retail it's copied in every invocation of RenderIceModelWithFlags.
  void Calculate(const CPoseAsTransforms& pose, CVertexMorphEffect* morphEffect, TConstVectorRef averagedNormals,
                 SSkinningWorkspace* workspace);
  void Draw(TConstVectorRef verts, TConstVectorRef normals, const CModelFlags& drawFlags);
  void Draw(const CModelFlags& drawFlags);
  void DoDrawCallback(const FCustomDraw& func) const;

  static void SetPointGeneratorFunc(FPointGenerator func) { g_PointGenFunc = std::move(func); }
  static void ClearPointGeneratorFunc() { g_PointGenFunc = nullptr; }
  static FPointGenerator g_PointGenFunc;
};

class CSkinnedModelWithAvgNormals : public CSkinnedModel {
  std::vector<zeus::CVector3f> x40_averagedNormals; // was rstl::auto_ptr<float[]>

public:
  CSkinnedModelWithAvgNormals(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo);
  ~CSkinnedModelWithAvgNormals() override = default;

  TConstVectorRef GetAveragedNormals() const { return &x40_averagedNormals; }
};

} // namespace metaforce
