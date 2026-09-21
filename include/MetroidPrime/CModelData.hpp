#ifndef _CMODELDATA
#define _CMODELDATA

#include "types.h"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"

class CAABox;
class CActorLights;
class CAnimRes;
class CFrustumPlanes;
class CModel;
class CModelFlags;
class CStateManager;
class CTexture;
class CSkinnedModel;
class CRandom16;
struct SThermalDrawContext;
struct SOneTextureDrawContext;
struct SFlatDrawContext;
struct SMultiLightingDrawContext;
struct SMultipassDrawContext;

class CStaticRes {
public:
  CStaticRes(CAssetId id, const CVector3f& scale) : x0_cmdlId(id), x4_scale(scale) {}

  const CAssetId GetId() const { return x0_cmdlId; }
  const CVector3f GetScale() const { return x4_scale; }

private:
  CAssetId x0_cmdlId;
  CVector3f x4_scale;
};

class CModelData {
public:
  int GetNumMaterialSets() const;
  enum EWhichModel {
    kWM_Normal,
    kWM_XRay,
    kWM_Thermal,
    kWM_ThermalHot,
  };

  // TODO these probably aren't real
  bool HasNormalModel() const { return x1c_normalModel; }

  CModelData();
  CModelData(const CAnimRes&);
  CModelData(const CStaticRes&);
  ~CModelData();

  CAdvancementDeltas AdvanceAnimation(float dt, CStateManager& mgr, TAreaId aid, bool advTree);
  void AdvanceParticles(const CTransform4f& xf, float dt, CStateManager& mgr);
  void RenderParticles(const CFrustumPlanes& planes) const;
  void RenderUnsortedParts(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                           const CModelFlags& flags) const;
  static void ThermalDraw(CSkinnedModel& model, const float* positions, const float* normals,
                          const CColor& mulColor, const CColor& addColor, const CModelFlags& flags);
  static void ThermalDraw(CSkinnedModel& model, const CColor& mulColor, const CColor& addColor,
                          const CModelFlags& flags);
  void RenderThermal(const CTransform4f& xf, const CColor& mulColor, const CColor& addColor,
                     const CModelFlags& flags) const;
  void Render(const CStateManager&, const CTransform4f&, const CActorLights*,
              const CModelFlags&) const;
  void Render(EWhichModel, const CTransform4f&, const CActorLights*, const CModelFlags&) const;
  void MultipassDraw(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                     const CModelFlags* flags, int count) const;
  void MultiLightingDraw(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                         const CColor& mulColor, const CColor& addColor) const;
  void FlatDraw(EWhichModel which, const CTransform4f& xf, bool unsortedOnly,
                const CModelFlags& flags) const;
  CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
  const TLockedToken< CModel >& PickStaticModel(EWhichModel which) const;
  void Touch(const CStateManager& mgr, int) const;
  void Touch(EWhichModel which, int) const;
  CAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool advTree);

  const CAnimData* GetAnimationData() const { return xc_animData.get(); }
  CAnimData* AnimationData() {
    CAnimData* ret = xc_animData.get();
    return ret;
  }
  CAABox GetBounds(const CTransform4f& xf) const;
  CAABox GetBounds() const;
  bool IsInFrustum(const CTransform4f& xf, const CFrustumPlanes& planes) const;
  bool IsLoaded(int shaderIdx) const;
  bool IsDefinitelyOpaque(EWhichModel which) const;

  CTransform4f GetLocatorTransformDynamic(const rstl::string& name,
                                          const CCharAnimTime* time) const;
  CTransform4f GetScaledLocatorTransformDynamic(const rstl::string& name,
                                                const CCharAnimTime* time) const;
  CTransform4f GetLocatorTransform(const rstl::string& name) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& name) const;

  bool HasAnimation() const { return !xc_animData.null(); }
  bool IsNull() const { return xc_animData.null() && !x1c_normalModel; }
  // TODO: maybe fake, but fixes CPatterned ctor codegen
  bool IsNotNull() const { return !xc_animData.null() || x1c_normalModel; }

  void SetXRayModel(const rstl::pair< CAssetId, CAssetId >& assets);
  void SetInfraModel(const rstl::pair< CAssetId, CAssetId >& assets);

  void SetAmbientColor(const CColor& color) { x18_ambientColor = color; }
  bool GetSortThermal() const { return x14_25_sortThermal; }
  void SetSortThermal(bool b) { x14_25_sortThermal = b; }

  CVector3f ScaleCopy() const { return x0_scale; }
  const CVector3f& GetScale() const { return x0_scale; }
  void SetScale(const CVector3f& scale) { x0_scale = scale; }

  bool GetIsLoop() const;
  void EnableLooping(bool enable);
  static CModelData CModelDataNull() { return CModelData(); }
  static EWhichModel GetRenderingModel(const CStateManager& mgr);
  float GetAnimationDuration(int anim) const;

  bool IsAnimating() const;
  bool HasModel(EWhichModel which) const;
  void DisintegrateDraw(const CStateManager&, const CTransform4f&, const CTexture&, const CColor&,
                        float) const;
  void DisintegrateDraw(EWhichModel which, const CTransform4f& xf, const CTexture& texture,
                        const CColor& color, float t) const;

private:
  static void ThermalDrawCallback(TModelPositions, TModelNormals, const SThermalDrawContext*);
  static void DisintegrateDrawCallback(TModelPositions, TModelNormals,
                                       const SOneTextureDrawContext*);
  static void FlatDrawCallback(TModelPositions, TModelNormals, const SFlatDrawContext*);
  static void MultiLightingDrawCallback(TModelPositions, TModelNormals, SMultiLightingDrawContext*);
  static void MultipassDrawCallback(TModelPositions, TModelNormals, const SMultipassDrawContext*);
  CVector3f x0_scale;
  rstl::auto_ptr< CAnimData > xc_animData;
  mutable bool x14_24_renderSorted : 1;
  bool x14_25_sortThermal : 1;
  CColor x18_ambientColor;
  rstl::optional_object< TLockedToken< CModel > > x1c_normalModel;
  rstl::optional_object< TLockedToken< CModel > > x2c_xrayModel;
  rstl::optional_object< TLockedToken< CModel > > x3c_infraModel;
};
CHECK_SIZEOF(CModelData, 0x4c)

#endif // _CMODELDATA
