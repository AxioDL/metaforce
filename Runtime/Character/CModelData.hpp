#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Graphics/CModel.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CActorLights;
class CAnimData;
class CCharAnimTime;
class CModel;
class CRandom16;
class CSkinnedModel;
class CStateManager;
struct CModelFlags;
struct SAdvancementDeltas;

class CStaticRes {
  CAssetId x0_cmdlId;
  zeus::CVector3f x4_scale;

public:
  constexpr CStaticRes() = default;
  CStaticRes(CAssetId id, const zeus::CVector3f& scale) : x0_cmdlId(id), x4_scale(scale) {}

  CAssetId GetId() const { return x0_cmdlId; }
  const zeus::CVector3f& GetScale() const { return x4_scale; }
  explicit operator bool() const { return x0_cmdlId.IsValid(); }
};

class CAnimRes {
  CAssetId x0_ancsId;
  s32 x4_charIdx = -1;
  zeus::CVector3f x8_scale;
  bool x14_canLoop = false;
  /* NOTE: x18_bodyType - Removed in retail */
  s32 x18_defaultAnim = -1; /* NOTE: used to be x1c in demo */
public:
  CAnimRes() = default;
  CAnimRes(CAssetId ancs, s32 charIdx, const zeus::CVector3f& scale, const s32 defaultAnim, bool loop)
  : x0_ancsId(ancs), x4_charIdx(charIdx), x8_scale(scale), x14_canLoop(loop), x18_defaultAnim(defaultAnim) {}

  CAssetId GetId() const { return x0_ancsId; }
  s32 GetCharacterNodeId() const { return x4_charIdx; }
  void SetCharacterNodeId(s32 id) { x4_charIdx = id; }
  const zeus::CVector3f& GetScale() const { return x8_scale; }
  bool CanLoop() const { return x14_canLoop; }
  void SetCanLoop(bool loop) { x14_canLoop = loop; }
  s32 GetDefaultAnim() const { return x18_defaultAnim; }
  void SetDefaultAnim(s32 anim) { x18_defaultAnim = anim; }
};

class CModelData {
  friend class CActor;
  zeus::CVector3f x0_scale;
  std::unique_ptr<CAnimData> x10_animData;
  bool x14_24_renderSorted : 1 = false;
  bool x14_25_sortThermal : 1 = false;
  zeus::CColor x18_ambientColor;

  // were rstl::optional_object<TCachedToken<CModel>>
  TLockedToken<CModel> x1c_normalModel;
  TLockedToken<CModel> x2c_xrayModel;
  TLockedToken<CModel> x3c_infraModel;

public:
  enum class EWhichModel { Normal, XRay, Thermal, ThermalHot };

  void SetSortThermal(bool sort) { x14_25_sortThermal = sort; }
  bool GetSortThermal() const { return x14_25_sortThermal; }

  ~CModelData();
  explicit CModelData(const CStaticRes& res);
  explicit CModelData(const CAnimRes& res);
  CModelData(CModelData&&) = default;
  CModelData& operator=(CModelData&&) = default;
  CModelData();
  static CModelData CModelDataNull();

  SAdvancementDeltas GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
  bool IsLoaded(int shaderIdx);
  static EWhichModel GetRenderingModel(const CStateManager& stateMgr);
  CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
  TLockedToken<CModel>& PickStaticModel(EWhichModel which);
  const TLockedToken<CModel>& PickStaticModel(EWhichModel which) const {
    return const_cast<CModelData*>(this)->PickStaticModel(which);
  }
  void SetXRayModel(const std::pair<CAssetId, CAssetId>& modelSkin);
  void SetInfraModel(const std::pair<CAssetId, CAssetId>& modelSkin);
  bool IsDefinitelyOpaque(EWhichModel) const;
  bool GetIsLoop() const;
  float GetAnimationDuration(int idx) const;
  void EnableLooping(bool enable);
  void AdvanceParticles(const zeus::CTransform& xf, float dt, CStateManager& stateMgr);
  zeus::CAABox GetBounds() const;
  zeus::CAABox GetBounds(const zeus::CTransform& xf) const;
  zeus::CTransform GetScaledLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const;
  zeus::CTransform GetScaledLocatorTransform(std::string_view name) const;
  zeus::CTransform GetLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const;
  zeus::CTransform GetLocatorTransform(std::string_view name) const;
  SAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool advTree);
  SAdvancementDeltas AdvanceAnimation(float dt, CStateManager& stateMgr, TAreaId aid, bool advTree);
  bool IsAnimating() const;
  bool IsInFrustum(const zeus::CTransform& xf, const zeus::CFrustum& frustum) const;
  void RenderParticles(const zeus::CFrustum& frustum) const;
  void Touch(EWhichModel, int shaderIdx);
  void Touch(const CStateManager& stateMgr, int shaderIdx);
  void RenderThermal(const zeus::CTransform& xf, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                     const CModelFlags& flags);
  void RenderUnsortedParts(EWhichModel, const zeus::CTransform& xf, const CActorLights* lights,
                           const CModelFlags& drawFlags);
  void Render(const CStateManager& stateMgr, const zeus::CTransform& xf, const CActorLights* lights,
              const CModelFlags& drawFlags);
  void Render(EWhichModel, const zeus::CTransform& xf, const CActorLights* lights, const CModelFlags& drawFlags);
  void FlatDraw(EWhichModel which, const zeus::CTransform& xf, bool unsortedOnly, const CModelFlags& flags);

  void MultiLightingDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                         const zeus::CColor& alphaColor, const zeus::CColor& additiveColor);
  void MultiPassDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                     const CModelFlags* flags, u32 count);
  void DisintegrateDraw(const CStateManager& mgr, const zeus::CTransform& xf, CTexture& tex,
                        const zeus::CColor& addColor, float t);
  void DisintegrateDraw(EWhichModel which, const zeus::CTransform& xf, CTexture& tex, const zeus::CColor& addColor,
                        float t);
  static void ThermalDraw(CSkinnedModel& model, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                          const CModelFlags& flags);
  static void ThermalDraw(CSkinnedModel& model, TConstVectorRef positions, TConstVectorRef normals,
                          const zeus::CColor& mulColor, const zeus::CColor& addColor, const CModelFlags& flags);

  CAnimData* GetAnimationData() { return x10_animData.get(); }
  const CAnimData* GetAnimationData() const { return x10_animData.get(); }
  const TLockedToken<CModel>& GetNormalModel() const { return x1c_normalModel; }
  const TLockedToken<CModel>& GetXRayModel() const { return x2c_xrayModel; }
  const TLockedToken<CModel>& GetThermalModel() const { return x3c_infraModel; }
  bool IsNull() const { return !x10_animData && !x1c_normalModel; }
  u32 GetNumMaterialSets() const;

  const zeus::CVector3f& GetScale() const { return x0_scale; }
  void SetScale(const zeus::CVector3f& scale) { x0_scale = scale; }
  bool HasAnimData() const { return x10_animData != nullptr; }
  bool HasNormalModel() const { return x1c_normalModel.HasReference(); }
  bool HasModel(EWhichModel which) const {
    if (x10_animData) {
      switch (which) {
      case EWhichModel::Normal:
        return true;
      case EWhichModel::XRay:
        return x10_animData->GetXRayModel() != nullptr;
      case EWhichModel::Thermal:
        return x10_animData->GetInfraModel() != nullptr;
      default:
        return false;
      }
    }

    switch (which) {
    case EWhichModel::Normal:
      return x1c_normalModel.HasReference();
    case EWhichModel::XRay:
      return x2c_xrayModel.HasReference();
    case EWhichModel::Thermal:
      return x3c_infraModel.HasReference();
    default:
      return false;
    }
  }
};

} // namespace metaforce
