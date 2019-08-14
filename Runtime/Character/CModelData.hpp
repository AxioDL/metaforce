#pragma once

#include "zeus/CVector3f.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CAnimData.hpp"
#include "Graphics/CModel.hpp"

namespace urde {
class CCharAnimTime;
class CStateManager;
class CActorLights;
struct CModelFlags;
class CRandom16;
class CAnimData;
class CModel;
class CSkinnedModel;
struct SAdvancementDeltas;

class CStaticRes {
  CAssetId x0_cmdlId = 0;
  zeus::CVector3f x4_scale;

public:
  CStaticRes(CAssetId id, const zeus::CVector3f& scale) : x0_cmdlId(id), x4_scale(scale) {}

  CAssetId GetId() const { return x0_cmdlId; }
  const zeus::CVector3f& GetScale() const { return x4_scale; }
  operator bool() const { return x0_cmdlId != 0; }
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
  void SetCanLoop(bool l) { x14_canLoop = l; }
  s32 GetDefaultAnim() const { return x18_defaultAnim; }
  void SetDefaultAnim(s32 anim) { x18_defaultAnim = anim; }
};

class CModelData {
  friend class CActor;
  zeus::CVector3f x0_scale;
  bool xc_ = false;
  std::unique_ptr<CAnimData> x10_animData;
  union {
    struct {
      bool x14_24_renderSorted : 1;
      bool x14_25_sortThermal : 1;
    };
    u32 _flags = 0;
  };
  zeus::CColor x18_ambientColor;

  TLockedToken<CModel> x1c_normalModel;
  TLockedToken<CModel> x2c_xrayModel;
  TLockedToken<CModel> x3c_infraModel;

  std::unique_ptr<CBooModel> m_normalModelInst;
  std::unique_ptr<CBooModel> m_xrayModelInst;
  std::unique_ptr<CBooModel> m_infraModelInst;

  int m_drawInstCount;

public:
  enum class EWhichModel { Normal, XRay, Thermal, ThermalHot };

  void SetSortThermal(bool v) { x14_25_sortThermal = v; }
  bool GetSortThermal() const { return x14_25_sortThermal; }

  ~CModelData();
  CModelData(const CStaticRes& res, int instCount = 1);
  CModelData(const CAnimRes& res, int instCount = 1);
  CModelData(CModelData&&) = default;
  CModelData& operator=(CModelData&&) = default;
  CModelData();
  static CModelData CModelDataNull();

  SAdvancementDeltas GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
  void Render(const CStateManager& stateMgr, const zeus::CTransform& xf, const CActorLights* lights,
              const CModelFlags& drawFlags) const;
  bool IsLoaded(int shaderIdx) const;
  static EWhichModel GetRenderingModel(const CStateManager& stateMgr);
  CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
  const std::unique_ptr<CBooModel>& PickStaticModel(EWhichModel which) const;
  void SetXRayModel(const std::pair<CAssetId, CAssetId>& modelSkin);
  void SetInfraModel(const std::pair<CAssetId, CAssetId>& modelSkin);
  bool IsDefinitelyOpaque(EWhichModel) const;
  bool GetIsLoop() const;
  float GetAnimationDuration(int) const;
  void EnableLooping(bool);
  void AdvanceParticles(const zeus::CTransform& xf, float, CStateManager& stateMgr);
  zeus::CAABox GetBounds() const;
  zeus::CAABox GetBounds(const zeus::CTransform& xf) const;
  zeus::CTransform GetScaledLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const;
  zeus::CTransform GetScaledLocatorTransform(std::string_view name) const;
  zeus::CTransform GetLocatorTransformDynamic(std::string_view name, const CCharAnimTime* time) const;
  zeus::CTransform GetLocatorTransform(std::string_view name) const;
  SAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16&, bool advTree);
  SAdvancementDeltas AdvanceAnimation(float dt, CStateManager& stateMgr, TAreaId aid, bool advTree);
  bool IsAnimating() const;
  bool IsInFrustum(const zeus::CTransform& xf, const zeus::CFrustum& frustum) const;
  void RenderParticles(const zeus::CFrustum& frustum) const;
  void Touch(EWhichModel, int shaderIdx) const;
  void Touch(const CStateManager& stateMgr, int shaderIdx) const;
  void RenderThermal(const zeus::CColor& mulColor, const zeus::CColor& addColor,
                     const CModelFlags& flags) const;
  void RenderThermal(const zeus::CTransform& xf, const zeus::CColor& mulColor, const zeus::CColor& addColor,
                     const CModelFlags& flags) const;
  void RenderUnsortedParts(EWhichModel, const zeus::CTransform& xf, const CActorLights* lights,
                           const CModelFlags& drawFlags) const;
  void Render(EWhichModel, const zeus::CTransform& xf, const CActorLights* lights, const CModelFlags& drawFlags) const;

  void InvSuitDraw(EWhichModel which, const zeus::CTransform& xf, const CActorLights* lights,
                   const zeus::CColor& color0, const zeus::CColor& color1);
  void DisintegrateDraw(const CStateManager& mgr, const zeus::CTransform& xf, const CTexture& tex,
                        const zeus::CColor& addColor, float t);
  void DisintegrateDraw(EWhichModel which, const zeus::CTransform& xf, const CTexture& tex,
                        const zeus::CColor& addColor, float t);

  CAnimData* AnimationData() { return x10_animData.get(); }
  const CAnimData* GetAnimationData() const { return x10_animData.get(); }
  const TLockedToken<CModel>& GetNormalModel() const { return x1c_normalModel; }
  const TLockedToken<CModel>& GetXRayModel() const { return x2c_xrayModel; }
  const TLockedToken<CModel>& GetThermalModel() const { return x3c_infraModel; }
  bool IsNull() const { return !x10_animData && !x1c_normalModel; }

  const zeus::CVector3f& GetScale() const { return x0_scale; }
  void SetScale(const zeus::CVector3f& scale) { x0_scale = scale; }
  bool HasAnimData() const { return x10_animData != nullptr; }
  bool HasNormalModel() const { return x1c_normalModel; }
  bool HasModel(EWhichModel which) const {
    if (x10_animData) {
      switch (which) {
      case EWhichModel::Normal:
        return true;
      case EWhichModel::XRay:
        return x10_animData->GetXRayModel().operator bool();
      case EWhichModel::Thermal:
        return x10_animData->GetInfraModel().operator bool();
      default:
        return false;
      }
    }

    switch (which) {
    case EWhichModel::Normal:
      return x1c_normalModel;
    case EWhichModel::XRay:
      return x2c_xrayModel;
    case EWhichModel::Thermal:
      return x3c_infraModel;
    default:
      return false;
    }
  }
};

} // namespace urde
