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

// Lambda instead of userdata pointer
using FCustomDraw = std::function<void(TVectorRef positions, TVectorRef normals)>;

class CSkinnedModel {
  TLockedToken<CModel> x4_model;
  TLockedToken<CSkinRules> x10_skinRules;
  TLockedToken<CCharLayoutInfo> x1c_layoutInfo;
  std::vector<zeus::CVector3f> x24_vertWorkspace;   // was rstl::auto_ptr<float[]>
  std::vector<zeus::CVector3f> x2c_normalWorkspace; // was rstl::auto_ptr<float[]>
  bool x34_owned = true;
  bool x35_disableWorkspaces = false;

public:
  enum class EDataOwnership { Unowned, Owned };
  CSkinnedModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules,
                const TLockedToken<CCharLayoutInfo>& layoutInfo /*, EDataOwnership ownership*/);
  CSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo);

  TLockedToken<CModel>& GetModel() { return x4_model; }
  const TLockedToken<CModel>& GetModel() const { return x4_model; }
  const TLockedToken<CSkinRules>& GetSkinRules() const { return x10_skinRules; }
  void SetLayoutInfo(const TLockedToken<CCharLayoutInfo>& inf) { x1c_layoutInfo = inf; }
  const TLockedToken<CCharLayoutInfo>& GetLayoutInfo() const { return x1c_layoutInfo; }

  void Calculate(const CPoseAsTransforms& pose, const std::optional<CVertexMorphEffect>& morphEffect,
                 const float* morphMagnitudes);
  void Draw(TVectorRef verts, TVectorRef normals, const CModelFlags& drawFlags);
  void Draw(const CModelFlags& drawFlags);
  void DoDrawCallback(const FCustomDraw& func) const;

  using FPointGenerator = void (*)(void* item, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
  static void SetPointGeneratorFunc(void* ctx, FPointGenerator func) {
    g_PointGenFunc = func;
    g_PointGenCtx = ctx;
  }
  static void ClearPointGeneratorFunc() { g_PointGenFunc = nullptr; }
  static FPointGenerator g_PointGenFunc;
  static void* g_PointGenCtx;
};

class CMorphableSkinnedModel : public CSkinnedModel {
  std::unique_ptr<float[]> x40_morphMagnitudes;

public:
  CMorphableSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo);
  const float* GetMorphMagnitudes() const { return x40_morphMagnitudes.get(); }
};

} // namespace metaforce
