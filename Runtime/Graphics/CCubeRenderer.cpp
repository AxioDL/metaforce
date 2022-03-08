#include "Runtime/Graphics/CCubeRenderer.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CDrawable.hpp"
#include "Runtime/Graphics/CMetroidModelInstance.hpp"
#include "Runtime/Graphics/CDrawablePlaneObject.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CParticleGen.hpp"
#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/Graphics/CCubeSurface.hpp"
#include "Runtime/Graphics/CCubeMaterial.hpp"

namespace metaforce {
static logvisor::Module Log("CCubeRenderer");

static rstl::reserved_vector<CDrawable, 512> sDataHolder;
static rstl::reserved_vector<rstl::reserved_vector<CDrawable*, 128>, 50> sBucketsHolder;
static rstl::reserved_vector<CDrawablePlaneObject, 8> sPlaneObjectDataHolder;
static rstl::reserved_vector<u16, 8> sPlaneObjectBucketHolder;

class Buckets {
  friend class CCubeRenderer;

  static inline rstl::reserved_vector<u16, 50> sBucketIndex;
  static inline rstl::reserved_vector<CDrawable, 512>* sData = nullptr;
  static inline rstl::reserved_vector<rstl::reserved_vector<CDrawable*, 128>, 50>* sBuckets = nullptr;
  static inline rstl::reserved_vector<CDrawablePlaneObject, 8>* sPlaneObjectData = nullptr;
  static inline rstl::reserved_vector<u16, 8>* sPlaneObjectBucket = nullptr;
  static constexpr std::array skWorstMinMaxDistance{99999.0f, -99999.0f};
  static inline std::array sMinMaxDistance{0.0f, 0.0f};

public:
  static void Clear();
  static void Sort();
  static void InsertPlaneObject(float closeDist, float farDist, const zeus::CAABox& aabb, bool invertTest,
                                const zeus::CPlane& plane, bool zOnly, EDrawableType dtype, void* data);
  static void Insert(const zeus::CVector3f& pos, const zeus::CAABox& aabb, EDrawableType dtype, void* data,
                     const zeus::CPlane& plane, u16 extraSort);
  static void Shutdown();
  static void Init();
};

void Buckets::Clear() {
  sData->clear();
  sBucketIndex.clear();
  sPlaneObjectData->clear();
  sPlaneObjectBucket->clear();
  for (rstl::reserved_vector<CDrawable*, 128>& bucket : *sBuckets) {
    bucket.clear();
  }
  sMinMaxDistance = skWorstMinMaxDistance;
}

void Buckets::Sort() {
  float delta = std::max(1.f, sMinMaxDistance[1] - sMinMaxDistance[0]);
  float pitch = 49.f / delta;
  for (auto it = sPlaneObjectData->begin(); it != sPlaneObjectData->end(); ++it)
    if (sPlaneObjectBucket->size() != sPlaneObjectBucket->capacity())
      sPlaneObjectBucket->push_back(s16(it - sPlaneObjectData->begin()));

  u32 precision = 50;
  if (sPlaneObjectBucket->size()) {
    std::sort(sPlaneObjectBucket->begin(), sPlaneObjectBucket->end(),
              [](u16 a, u16 b) { return (*sPlaneObjectData)[a].GetDistance() < (*sPlaneObjectData)[b].GetDistance(); });
    precision = 50 / u32(sPlaneObjectBucket->size() + 1);
    pitch = 1.f / (delta / float(precision - 2));

    s32 accum = 0;
    for (u16 idx : *sPlaneObjectBucket) {
      ++accum;
      CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[idx];
      planeObj.x24_targetBucket = u16(precision * accum);
    }
  }

  for (CDrawable& drawable : *sData) {
    s32 slot;
    float relDist = drawable.GetDistance() - sMinMaxDistance[0];
    if (sPlaneObjectBucket->empty()) {
      slot = zeus::clamp(1, s32(relDist * pitch), 49);
    } else {
      slot = zeus::clamp(0, s32(relDist * pitch), s32(precision) - 2);
      for (u16 idx : *sPlaneObjectBucket) {
        CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[idx];
        bool partial, full;
        if (planeObj.x3c_25_zOnly) {
          partial = drawable.GetBounds().max.z() > planeObj.GetPlane().d();
          full = drawable.GetBounds().min.z() > planeObj.GetPlane().d();
        } else {
          partial = planeObj.GetPlane().pointToPlaneDist(
                        drawable.GetBounds().closestPointAlongVector(planeObj.GetPlane().normal())) > 0.f;
          full = planeObj.GetPlane().pointToPlaneDist(
                     drawable.GetBounds().furthestPointAlongVector(planeObj.GetPlane().normal())) > 0.f;
        }
        bool cont;
        if (drawable.GetType() == EDrawableType::Particle)
          cont = planeObj.x3c_24_invertTest ? !partial : full;
        else
          cont = planeObj.x3c_24_invertTest ? (!partial || !full) : (partial || full);
        if (!cont)
          break;
        slot += precision;
      }
    }

    if (slot == -1)
      slot = 49;
    rstl::reserved_vector<CDrawable*, 128>& bucket = (*sBuckets)[slot];
    if (bucket.size() < bucket.capacity())
      bucket.push_back(&drawable);
    // else
    //    Log.report(logvisor::Fatal, FMT_STRING("Full bucket!!!"));
  }

  u16 bucketIdx = u16(sBuckets->size());
  for (auto it = sBuckets->rbegin(); it != sBuckets->rend(); ++it) {
    --bucketIdx;
    sBucketIndex.push_back(bucketIdx);
    rstl::reserved_vector<CDrawable*, 128>& bucket = *it;
    if (bucket.size()) {
      std::sort(bucket.begin(), bucket.end(), [](CDrawable* a, CDrawable* b) {
        if (a->GetDistance() == b->GetDistance())
          return a->GetExtraSort() > b->GetExtraSort();
        return a->GetDistance() > b->GetDistance();
      });
    }
  }

  for (auto it = sPlaneObjectBucket->rbegin(); it != sPlaneObjectBucket->rend(); ++it) {
    CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[*it];
    rstl::reserved_vector<CDrawable*, 128>& bucket = (*sBuckets)[planeObj.x24_targetBucket];
    bucket.push_back(&planeObj);
  }
}

void Buckets::InsertPlaneObject(float closeDist, float farDist, const zeus::CAABox& aabb, bool invertTest,
                                const zeus::CPlane& plane, bool zOnly, EDrawableType dtype, void* data) {
  if (sPlaneObjectData->size() == sPlaneObjectData->capacity()) {
    return;
  }
  sPlaneObjectData->emplace_back(dtype, closeDist, farDist, aabb, invertTest, plane, zOnly, data);
}

void Buckets::Insert(const zeus::CVector3f& pos, const zeus::CAABox& aabb, EDrawableType dtype, void* data,
                     const zeus::CPlane& plane, u16 extraSort) {
  if (sData->size() == sData->capacity()) {
    Log.report(logvisor::Fatal, FMT_STRING("Rendering buckets filled to capacity"));
    return;
  }

  const float dist = plane.pointToPlaneDist(pos);
  sData->emplace_back(dtype, extraSort, dist, aabb, data);
  sMinMaxDistance[0] = std::min(sMinMaxDistance[0], dist);
  sMinMaxDistance[1] = std::max(sMinMaxDistance[1], dist);
}

void Buckets::Shutdown() {
  sData = nullptr;
  sBuckets = nullptr;
  sPlaneObjectData = nullptr;
  sPlaneObjectBucket = nullptr;
}

void Buckets::Init() {
  sData = &sDataHolder;
  sBuckets = &sBucketsHolder;
  sBuckets->resize(50);
  sPlaneObjectData = &sPlaneObjectDataHolder;
  sPlaneObjectBucket = &sPlaneObjectBucketHolder;
  sMinMaxDistance = skWorstMinMaxDistance;
}

CCubeRenderer::CAreaListItem::CAreaListItem(const std::vector<CMetroidModelInstance>* geom,
                                            const CAreaRenderOctTree* octTree,
                                            std::unique_ptr<std::vector<TCachedToken<CTexture>>>&& textures,
                                            std::unique_ptr<std::vector<std::unique_ptr<CCubeModel>>>&& models,
                                            s32 areaIdx)
: x0_geometry(geom)
, x4_octTree(octTree)
, x8_textures(std::move(textures))
, x10_models(std::move(models))
, x18_areaIdx(areaIdx) {}

CCubeRenderer::CCubeRenderer(IObjectStore& store, IFactory& resFac) : x8_factory(resFac), xc_store(store) {
  void* data = xe4_blackTex.Lock();
  memset(data, 0, 32);
  xe4_blackTex.UnLock();
  GenerateReflectionTex();
  GenerateFogVolumeRampTex();
  GenerateSphereRampTex();
  LoadThermoPalette();
  g_Renderer = this;
  Buckets::Init();
  // GX draw sync
}

CCubeRenderer::~CCubeRenderer() { g_Renderer = nullptr; }

void CCubeRenderer::GenerateReflectionTex() {}
void CCubeRenderer::GenerateFogVolumeRampTex() {}
void CCubeRenderer::GenerateSphereRampTex() {}
void CCubeRenderer::LoadThermoPalette() {}
void CCubeRenderer::ReallyDrawPhazonSuitIndirectEffect(const zeus::CColor& vertColor, const CTexture& maskTex,
                                                       const CTexture& indTex, const zeus::CColor& modColor,
                                                       float scale, float offX, float offY) {}
void CCubeRenderer::ReallyDrawPhazonSuitEffect(const zeus::CColor& modColor, const CTexture& maskTex) {}
void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float blurRadius, float f2, const TLockedToken<CTexture>& indTex) {}

void CCubeRenderer::AddWorldSurfaces(CCubeModel& model) {
  for (auto* it = model.GetFirstSortedSurface(); it != nullptr; it = it->GetNextSurface()) {
    auto mat = model.GetMaterialByIndex(it->GetMaterialIndex());
    auto blend = mat.GetCompressedBlend();
    auto bounds = it->GetBounds();
    auto pos = bounds.closestPointAlongVector(xb0_viewPlane.normal());
    Buckets::Insert(pos, bounds, EDrawableType::WorldSurface, it, xb0_viewPlane, static_cast<u16>(blend == 0x50004));
  }
}
void CCubeRenderer::AddStaticGeometry(const std::vector<CMetroidModelInstance>* geometry,
                                      const CAreaRenderOctTree* octTree, s32 areaIdx) {
  auto search = FindStaticGeometry(geometry);
  if (search == x1c_areaListItems.end()) {
    auto textures = std::make_unique<std::vector<TCachedToken<CTexture>>>();
    auto models = std::make_unique<std::vector<std::unique_ptr<CCubeModel>>>();
    if (!geometry->empty()) {
      CCubeModel::MakeTexturesFromMats((*geometry)[0].GetMaterialPointer(), *textures.get(), &xc_store, false);
      models->reserve(geometry->size());
      s32 instIdx = 0;
      for (const CMetroidModelInstance& inst : *geometry) {
        models->emplace_back(
            std::make_unique<CCubeModel>(const_cast<std::vector<CCubeSurface>*>(inst.GetSurfaces()), textures.get(),
                                         const_cast<u8*>(inst.GetMaterialPointer()),
                                         const_cast<std::vector<zeus::CVector3f>*>(inst.GetVertexPointer()),
                                         const_cast<std::vector<zeus::CColor>*>(inst.GetColorPointer()),
                                         const_cast<std::vector<zeus::CVector3f>*>(inst.GetNormalPointer()),
                                         const_cast<std::vector<aurora::Vec2<float>>*>(inst.GetTCPointer()),
                                         const_cast<std::vector<aurora::Vec2<float>>*>(inst.GetPackedTCPointer()),
                                         inst.GetBoundingBox(), inst.GetFlags(), false, instIdx));
        ++instIdx;
      }
    }
    x1c_areaListItems.emplace_back(geometry, octTree, std::move(textures), std::move(models), areaIdx);
  }
}

void CCubeRenderer::EnablePVS(const CPVSVisSet& set, u32 areaIdx) {
  if (!xdc_) {
    xc8_pvs.emplace(set);
    xdc_ = true;
  } else {
    xc8_pvs.emplace(set);
  }

  xe0_pvsAreaIdx = areaIdx;
}

void CCubeRenderer::DisablePVS() { xc8_pvs.reset(); }

void CCubeRenderer::RemoveStaticGeometry(const std::vector<CMetroidModelInstance>* geometry) {
  auto search = FindStaticGeometry(geometry);
  if (search != x1c_areaListItems.end()) {
    x1c_areaListItems.erase(search);
  }
}

void CCubeRenderer::DrawUnsortedGeometry(s32 areaIdx, s32 mask, s32 targetMask, bool shadowRender) {
  SetupRendererStates(true);
  CModelFlags flags;
  CAreaListItem* lastOctreeItem = nullptr;

  for (CAreaListItem& item : x1c_areaListItems) {
    if (areaIdx != -1 && item.x18_areaIdx != areaIdx) {
      continue;
    }

    if (item.x4_octTree != nullptr) {
      lastOctreeItem = &item;
    }

    CPVSVisSet* pvs = nullptr;
    if (xc8_pvs) {
      pvs = &*xc8_pvs;
    }

    if (xe0_pvsAreaIdx != item.x18_areaIdx) {
      pvs = nullptr;
    }

    u32 idx = 0;
    for (auto it = item.x10_models->begin(); it != item.x10_models->end(); ++it, ++idx) {
      const auto& model = *it;
      if (pvs != nullptr) {
        bool vis = pvs->GetVisible(idx) != EPVSVisSetState::EndOfTree;
        switch (xc0_pvsMode) {
        case EPVSMode::PVS: {
          if (!vis) {
            model->SetVisible(false);
            continue;
          }
          break;
        }
        case EPVSMode::PVSAndMask: {
          if (!vis && (model->GetFlags() & mask) != targetMask) {
            model->SetVisible(false);
            continue;
          }
          break;
        }
        default:
          break;
        }
      }

      if ((model->GetFlags() & mask) != targetMask) {
        model->SetVisible(false);
        continue;
      }

      if (!x44_frustumPlanes.aabbFrustumTest(model->GetBounds())) {
        model->SetVisible(false);
        continue;
      }

      if (x318_25_drawWireframe) {
        model->SetVisible(false);
        HandleUnsortedModelWireframe(lastOctreeItem, *model);
        continue;
      }

      model->SetVisible(true);
      HandleUnsortedModel(lastOctreeItem, *model, flags);
    }
  }

  SetupCGraphicsState();
}

void CCubeRenderer::DrawSortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) {
  SetupRendererStates(true);
  const CAreaListItem* item = nullptr;
  for (const auto& areaListItem : x1c_areaListItems) {
    if (areaIdx == -1 || areaIdx == areaListItem.x18_areaIdx) {
      if (areaListItem.x4_octTree != nullptr) {
        item = &areaListItem;
      }

      for (const auto& model : *areaListItem.x10_models) {
        if (model->IsVisible()) {
          AddWorldSurfaces(*model);
        }
      }
    }
  }
  Buckets::Sort();
  RenderBucketItems(item);
  SetupCGraphicsState();
  DrawRenderBucketsDebug();
  Buckets::Clear();
}

void CCubeRenderer::DrawStaticGeometry(s32 areaIdx, s32 mask, s32 targetMask) {
  DrawUnsortedGeometry(areaIdx, mask, targetMask);
  DrawSortedGeometry(areaIdx, mask, targetMask);
}

void CCubeRenderer::DrawAreaGeometry(s32 areaIdx, s32 mask, s32 targetMask) {
  x318_30_inAreaDraw = true;
  SetupRendererStates(true);
  CModelFlags flags;

  for (CAreaListItem& item : x1c_areaListItems) {
    if (areaIdx != -1 || item.x18_areaIdx == areaIdx) {
      CPVSVisSet* pvs = xc8_pvs ? &*xc8_pvs : nullptr;
      if (xe0_pvsAreaIdx != item.x18_areaIdx) {
        pvs = nullptr;
      }
      s32 modelIdx = 0;
      for (auto it = item.x10_models->begin(); it != item.x10_models->end(); ++it, ++modelIdx) {
        const auto& model = *it;
        if (pvs != nullptr) {
          bool visible = pvs->GetVisible(modelIdx) != EPVSVisSetState::EndOfTree;
          if ((xc0_pvsMode == EPVSMode::PVS && !visible) || (xc0_pvsMode == EPVSMode::PVSAndMask && visible)) {
            continue;
          }
        }
        if ((model->GetFlags() & mask) != targetMask) {
          continue;
        }
        if (!x44_frustumPlanes.aabbFrustumTest(model->GetBounds())) {
          continue;
        }

        for (const auto* surf = model->GetFirstUnsortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
          model->DrawSurface(*surf, CModelFlags(0, 0, 3, zeus::skWhite));
        }
        for (const auto* surf = model->GetFirstSortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
          model->DrawSurface(*surf, CModelFlags(0, 0, 3, zeus::skWhite));
        }
      }
    }
  }

  x318_30_inAreaDraw = false;
}

void CCubeRenderer::RenderBucketItems(const CAreaListItem* item) {
  CCubeModel* lastModel = nullptr;
  EDrawableType lastDrawableType = EDrawableType::Invalid;
  for (u16 idx : Buckets::sBucketIndex) {
    rstl::reserved_vector<CDrawable*, 128>& bucket = (*Buckets::sBuckets)[idx];
    for (CDrawable* drawable : bucket) {
      EDrawableType type = drawable->GetType();
      switch (type) {
      case EDrawableType::Particle: {
        if (lastDrawableType != EDrawableType::Particle) {
          SetupCGraphicsState();
        }

        static_cast<CParticleGen*>(drawable->GetData())->Render();
        break;
      }
      case EDrawableType::WorldSurface: {
        if (lastDrawableType != EDrawableType::WorldSurface) {
          SetupRendererStates(false);
          lastModel = nullptr;
        }

        auto* surface = static_cast<CCubeSurface*>(drawable->GetData());
        auto* model = surface->GetParent();
        if (model != lastModel) {
          model->SetArraysCurrent();
          ActivateLightsForModel(item, *model);
        }
        model->DrawSurface(*surface, CModelFlags(0, 0, 1, zeus::skWhite));
        break;
      }
      default: {
        if (type != lastDrawableType) {
          CCubeMaterial::EnsureTevsDirect();
        }
        if (xa8_drawableCallback != nullptr) {
          xa8_drawableCallback(drawable->GetData(), xac_drawableCallbackUserData, s32(drawable->GetType()) - 2);
        }
        break;
      }
      }
      lastDrawableType = type;
    }
  }
}
void CCubeRenderer::PostRenderFogs() {}
void CCubeRenderer::SetModelMatrix(const zeus::CTransform& xf) { CGraphics::SetModelMatrix(xf); }

void CCubeRenderer::HandleUnsortedModel(CAreaListItem* areaItem, CCubeModel& model, const CModelFlags& flags) {
  if (model.GetFirstUnsortedSurface() == nullptr) {
    return;
  }
  model.SetArraysCurrent();
  ActivateLightsForModel(areaItem, model);
  for (auto* it = model.GetFirstUnsortedSurface(); it != nullptr; it = it->GetNextSurface()) {
    model.DrawSurface(*it, CModelFlags(0, 0, 3, zeus::skWhite));
  }
}

void CCubeRenderer::HandleUnsortedModelWireframe(CAreaListItem* areaItem, CCubeModel& model) {
  model.SetArraysCurrent();
  ActivateLightsForModel(areaItem, model);
  for (auto* it = model.GetFirstUnsortedSurface(); it != nullptr; it = it->GetNextSurface()) {
    model.DrawSurfaceWireframe(*it);
  }
  for (auto* it = model.GetFirstSortedSurface(); it != nullptr; it = it->GetNextSurface()) {
    model.DrawSurfaceWireframe(*it);
  }
}

void CCubeRenderer::ActivateLightsForModel(const CAreaListItem* areaItem, CCubeModel& model) {}
void CCubeRenderer::AddParticleGen(CParticleGen& gen) {
  auto bounds = gen.GetBounds();

  if (bounds) {
    auto closestPoint = bounds->closestPointAlongVector(xb0_viewPlane.normal());
    Buckets::Insert(closestPoint, *bounds, EDrawableType::Particle, reinterpret_cast<void*>(&gen), xb0_viewPlane, 0);
  }
}

void CCubeRenderer::AddParticleGen(CParticleGen& gen, const zeus::CVector3f& pos, const zeus::CAABox& bounds) {
  Buckets::Insert(pos, bounds, EDrawableType::Particle, reinterpret_cast<void*>(&gen), xb0_viewPlane, 0);
}

void CCubeRenderer::AddPlaneObject(void* obj, const zeus::CAABox& aabb, const zeus::CPlane& plane, s32 type) {

  const auto closestPoint = aabb.closestPointAlongVector(xb0_viewPlane.normal());
  const auto closestDist = xb0_viewPlane.pointToPlaneDist(closestPoint);
  const auto furthestPoint = aabb.furthestPointAlongVector(xb0_viewPlane.normal());
  const auto furthestDist = xb0_viewPlane.pointToPlaneDist(furthestPoint);

  if (closestDist >= 0.f || furthestDist >= 0.f) {
    const bool zOnly = plane.normal() == zeus::skUp;
    const bool invertTest = zOnly ? CGraphics::g_GXModelView.origin.z() >= plane.d()
                                  : plane.pointToPlaneDist(CGraphics::g_GXModelView.origin) >= 0.f;
    Buckets::InsertPlaneObject(closestDist, furthestDist, aabb, invertTest, plane, zOnly, EDrawableType(type + 2), obj);
  }
}

void CCubeRenderer::AddDrawable(void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, s32 mode,
                                IRenderer::EDrawableSorting sorting) {
  if (sorting == EDrawableSorting::UnsortedCallback) {
    xa8_drawableCallback(obj, xac_drawableCallbackUserData, mode);
  } else {
    Buckets::Insert(pos, aabb, EDrawableType(mode + 2), obj, xb0_viewPlane, 0);
  }
}

void CCubeRenderer::SetDrawableCallback(IRenderer::TDrawableCallback cb, void* ctx) {
  xa8_drawableCallback = cb;
  xac_drawableCallbackUserData = ctx;
}

void CCubeRenderer::SetWorldViewpoint(const zeus::CTransform& xf) {
  CGraphics::SetViewPointMatrix(xf);
  auto front = xf.frontVector();
  xb0_viewPlane = zeus::CPlane(front, front.dot(xf.origin));
}

void CCubeRenderer::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, aspect, znear, zfar);
}

void CCubeRenderer::SetPerspective(float fovy, float width, float height, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, width / height, znear, zfar);
}

std::pair<zeus::CVector2f, zeus::CVector2f> CCubeRenderer::SetViewportOrtho(bool centered, float znear, float zfar) {
  auto left = static_cast<float>(centered ? CGraphics::GetViewportLeft() - CGraphics::GetViewportWidth() / 2
                                          : CGraphics::GetViewportLeft());
  auto top = static_cast<float>(centered ? CGraphics::GetViewportTop() - CGraphics::GetViewportHeight() / 2
                                         : CGraphics::GetViewportHeight());
  auto right = static_cast<float>(CGraphics::GetViewportLeft() +
                                  (centered ? CGraphics::GetViewportWidth() / 2 : CGraphics::GetViewportWidth()));
  auto bottom = static_cast<float>(CGraphics::GetViewportTop() +
                                   (centered ? CGraphics::GetViewportHeight() / 2 : CGraphics::GetViewportHeight()));
  CGraphics::SetOrtho(left, right, top, bottom, znear, zfar);
  CGraphics::SetViewPointMatrix({});
  CGraphics::SetModelMatrix({});
  return {{left, top}, {right, bottom}};
}

void CCubeRenderer::SetClippingPlanes(const zeus::CFrustum& frustum) { x44_frustumPlanes = frustum; }

void CCubeRenderer::SetViewport(s32 left, s32 bottom, s32 width, s32 height) {
  CGraphics::SetViewport(left, bottom, width, height);
  CGraphics::SetScissor(left, bottom, width, height);
}

void CCubeRenderer::BeginScene() {
  CGraphics::SetUseVideoFilter(true);
  CGraphics::SetViewport(0, 0, CGraphics::g_Viewport.x8_width, CGraphics::g_Viewport.xc_height);

  CGraphics::SetClearColor(zeus::skClear);
  CGraphics::SetCullMode(ERglCullMode::Front);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                          ERglLogicOp::Clear);
  CGraphics::SetPerspective(75.f, CGraphics::g_Viewport.x8_width / CGraphics::g_Viewport.xc_height, 1.f, 4096.f);
  CGraphics::SetModelMatrix(zeus::CTransform());
  if (x310_phazonSuitMaskCountdown != 0) {
    --x310_phazonSuitMaskCountdown;
    if (x310_phazonSuitMaskCountdown == 0) {
      x314_phazonSuitMask.reset();
    }
  }

  if (!x318_31_persistRGBA6) {
    x318_26_requestRGBA6 = false;
  }

  // GXSetPixelFmt(x318_26_requestRGBA6, GX_ZC_LINEAR);
  aurora::gfx::set_alpha_update(true);
  aurora::gfx::set_dst_alpha(true, 0.f);
  CGraphics::BeginScene();
}

void CCubeRenderer::EndScene() {
  x318_31_persistRGBA6 = !CGraphics::g_IsBeginSceneClearFb;
  CGraphics::EndScene();

  if (x2dc_reflectionAge < 2) {
    ++x2dc_reflectionAge;
  } else {
    x14c_reflectionTex.reset();
  };
}

void CCubeRenderer::SetDebugOption(IRenderer::EDebugOption option, s32 value) {
  if (option == EDebugOption::PVSState) {
    xc8_pvs->SetState(EPVSVisSetState(value));
  } else if (option == EDebugOption::PVSMode) {
    xc0_pvsMode = EPVSMode(value);
  } else if (option == EDebugOption::FogDisabled) {
    x318_28_disableFog = true;
  }
}

void CCubeRenderer::BeginPrimitive(IRenderer::EPrimitiveType type, s32 nverts) {
  x18_primVertCount = nverts;
  CGraphics::StreamBegin(GX::Primitive(type));
}
void CCubeRenderer::BeginLines(s32 nverts) { BeginPrimitive(EPrimitiveType::Lines, nverts); }
void CCubeRenderer::BeginLineStrip(s32 nverts) { BeginPrimitive(EPrimitiveType::LineStrip, nverts); }
void CCubeRenderer::BeginTriangles(s32 nverts) { BeginPrimitive(EPrimitiveType::Triangles, nverts); }
void CCubeRenderer::BeginTriangleStrip(s32 nverts) { BeginPrimitive(EPrimitiveType::TriangleStrip, nverts); }
void CCubeRenderer::BeginTriangleFan(s32 nverts) { BeginPrimitive(EPrimitiveType::TriangleFan, nverts); }
void CCubeRenderer::PrimVertex(const zeus::CVector3f& vertex) {
  --x18_primVertCount;
  CGraphics::StreamColor(x2e0_primColor);
  CGraphics::StreamNormal(x2e4_primNormal);
  CGraphics::StreamVertex(vertex);
}
void CCubeRenderer::PrimNormal(const zeus::CVector3f& normal) { x2e4_primNormal = normal; }
void CCubeRenderer::PrimColor(float r, float g, float b, float a) { PrimColor({r, g, b, a}); }
void CCubeRenderer::PrimColor(const zeus::CColor& color) { x2e0_primColor = color; }
void CCubeRenderer::EndPrimitive() {
  while (x18_primVertCount > 0) {
    PrimVertex(zeus::skZero3f);
  }
  CGraphics::StreamEnd();
}
void CCubeRenderer::SetAmbientColor(const zeus::CColor& color) { CGraphics::SetAmbientColor(color); }
void CCubeRenderer::DrawString(const char* string, s32 x, s32 y) { x10_font.DrawString(string, x, y, zeus::skWhite); }

u32 CCubeRenderer::GetFPS() { return CGraphics::GetFPS(); }
void CCubeRenderer::CacheReflection(IRenderer::TReflectionCallback cb, void* ctx, bool clearAfter) {}
void CCubeRenderer::DrawSpaceWarp(const zeus::CVector3f& pt, float strength) {}
void CCubeRenderer::DrawThermalModel(const CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                                     TVectorRef positions, TVectorRef normals, const CModelFlags& flags) {}
void CCubeRenderer::DrawModelDisintegrate(const CModel& model, const CTexture& tex, const zeus::CColor& color,
                                          TVectorRef positions, TVectorRef normals) {}
void CCubeRenderer::DrawModelFlat(const CModel& model, const CModelFlags& flags, bool unsortedOnly) {}
void CCubeRenderer::SetWireframeFlags(s32 flags) {}
void CCubeRenderer::SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) {}
void CCubeRenderer::RenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb,
                                    const TLockedToken<CModel>* model, const CSkinnedModel* sModel) {}
void CCubeRenderer::SetThermal(bool thermal, float level, const zeus::CColor& color) {}
void CCubeRenderer::SetThermalColdScale(float scale) {}
void CCubeRenderer::DoThermalBlendCold() {}
void CCubeRenderer::DoThermalBlendHot() {}
u32 CCubeRenderer::GetStaticWorldDataSize() { return 0; }

void CCubeRenderer::SetGXRegister1Color(const zeus::CColor& color) {
  aurora::gfx::set_tev_reg_color(GX::TevRegID::TEVREG1, color);
}

void CCubeRenderer::SetWorldLightFadeLevel(float level) { x2fc_tevReg1Color = zeus::CColor(level, level, level, 1.f); }

void CCubeRenderer::PrepareDynamicLights(const std::vector<CLight>& lights) {}
void CCubeRenderer::AllocatePhazonSuitMaskTexture() {}
void CCubeRenderer::DrawPhazonSuitIndirectEffect(const zeus::CColor& nonIndirectMod,
                                                 const TLockedToken<CTexture>& indTex, const zeus::CColor& indirectMod,
                                                 float blurRadius, float scale, float offX, float offY) {}
void CCubeRenderer::DrawXRayOutline(const zeus::CAABox& aabb) {}

std::list<CCubeRenderer::CAreaListItem>::iterator
CCubeRenderer::FindStaticGeometry(const std::vector<CMetroidModelInstance>* geometry) {
  return std::find_if(x1c_areaListItems.begin(), x1c_areaListItems.end(),
                      [&](const CAreaListItem& item) { return item.x0_geometry == geometry; });
}

void CCubeRenderer::FindOverlappingWorldModels(std::vector<u32>& modelBits, const zeus::CAABox& aabb) const {}
s32 CCubeRenderer::DrawOverlappingWorldModelIDs(s32 alphaVal, const std::vector<u32>& modelBits,
                                                const zeus::CAABox& aabb) {
  SetupRendererStates(true);

  return 0;
}
void CCubeRenderer::DrawOverlappingWorldModelShadows(s32 alphaVal, const std::vector<u32>& modelBits,
                                                     const zeus::CAABox& aabb, float alpha) {}

void CCubeRenderer::SetupCGraphicsState() {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix({});
  CTevCombiners::ResetStates();
  CGraphics::SetAmbientColor({0.4f});
  // CGX::SetChanMatColor(EChannelId::Channel0, GX::Color{0xFFFFFFFF});
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
  // CGX::SetChanCtrl(EChannelId::Channel1, false, GX::SRC_REG, GX::LIGHT_NULL, GX::DF_NONE, GX::AF_NONE);
  CCubeMaterial::EnsureTevsDirect();
}

void CCubeRenderer::SetupRendererStates(bool depthWrite) {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix({});
  CGraphics::SetAmbientColor(zeus::skBlack);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, depthWrite);
  CCubeMaterial::ResetCachedMaterials();
  aurora::gfx::set_tev_reg_color(GX::TEVREG1, x2fc_tevReg1Color);
}
} // namespace metaforce
