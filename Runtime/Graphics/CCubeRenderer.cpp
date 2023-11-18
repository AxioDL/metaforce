#include "Runtime/Graphics/CCubeRenderer.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeMaterial.hpp"
#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/Graphics/CCubeSurface.hpp"
#include "Runtime/Graphics/CDrawable.hpp"
#include "Runtime/Graphics/CDrawablePlaneObject.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CMetroidModelInstance.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/Particle/CParticleGen.hpp"
#include "Runtime/Particle/CDecal.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/CDvdFile.hpp"

#include <utility>

namespace metaforce {
static logvisor::Module Log("CCubeRenderer");

/* TODO: This is to fix some areas exceeding the max drawable count, the proper number is 128 drawables per bucket */
// using BucketHolderType = rstl::reserved_vector<CDrawable*, 128>;
using BucketHolderType = rstl::reserved_vector<CDrawable*, 132>;
static rstl::reserved_vector<CDrawable, 512> sDataHolder;
static rstl::reserved_vector<BucketHolderType, 50> sBucketsHolder;
static rstl::reserved_vector<CDrawablePlaneObject, 8> sPlaneObjectDataHolder;
static rstl::reserved_vector<u16, 8> sPlaneObjectBucketHolder;

class Buckets {
  friend class CCubeRenderer;

  static inline rstl::reserved_vector<u16, 50> sBucketIndex;
  static inline rstl::reserved_vector<CDrawable, 512>* sData = nullptr;
  static inline rstl::reserved_vector<BucketHolderType, 50>* sBuckets = nullptr;
  static inline rstl::reserved_vector<CDrawablePlaneObject, 8>* sPlaneObjectData = nullptr;
  static inline rstl::reserved_vector<u16, 8>* sPlaneObjectBucket = nullptr;
  static constexpr std::array skWorstMinMaxDistance{99999.0f, -99999.0f};
  static inline std::array sMinMaxDistance{99999.0f, -99999.0f};

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
  for (BucketHolderType& bucket : *sBuckets) {
    bucket.clear();
  }
  sMinMaxDistance = skWorstMinMaxDistance;
}

void Buckets::Sort() {
  float delta = std::max(1.f, sMinMaxDistance[1] - sMinMaxDistance[0]);
  float pitch = 49.f / delta;
  for (auto it = sPlaneObjectData->begin(); it != sPlaneObjectData->end(); ++it) {
    if (sPlaneObjectBucket->size() != sPlaneObjectBucket->capacity()) {
      sPlaneObjectBucket->push_back(s16(it - sPlaneObjectData->begin()));
    }
  }

  u32 precision = 50;
  if (!sPlaneObjectBucket->empty()) {
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
    s32 slot = -1;
    float relDist = drawable.GetDistance() - sMinMaxDistance[0];
    if (sPlaneObjectBucket->empty()) {
      slot = zeus::clamp(1, s32(relDist * pitch), 49);
    } else {
      slot = zeus::clamp(0, s32(relDist * pitch), s32(precision) - 2);
      for (u16 idx : *sPlaneObjectBucket) {
        CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[idx];
        bool partial = false;
        bool full = false;
        if (planeObj.x3c_25_zOnly) {
          partial = drawable.GetBounds().max.z() > planeObj.GetPlane().d();
          full = drawable.GetBounds().min.z() > planeObj.GetPlane().d();
        } else {
          partial = planeObj.GetPlane().pointToPlaneDist(
                        drawable.GetBounds().closestPointAlongVector(planeObj.GetPlane().normal())) > 0.f;
          full = planeObj.GetPlane().pointToPlaneDist(
                     drawable.GetBounds().furthestPointAlongVector(planeObj.GetPlane().normal())) > 0.f;
        }
        bool cont = false;
        if (drawable.GetType() == EDrawableType::Particle) {
          cont = planeObj.x3c_24_invertTest ? !partial : full;
        } else {
          cont = planeObj.x3c_24_invertTest ? (!partial || !full) : (partial || full);
        }
        if (!cont) {
          break;
        }
        slot += s32(precision);
      }
    }

    if (slot == -1) {
      slot = 49;
    }
    BucketHolderType& bucket = (*sBuckets)[slot];
    if (bucket.size() < bucket.capacity()) {
      bucket.push_back(&drawable);
    }
    // else
    //    Log.report(logvisor::Fatal, FMT_STRING("Full bucket!!!"));
  }

  u16 bucketIdx = u16(sBuckets->size());
  for (auto it = sBuckets->rbegin(); it != sBuckets->rend(); ++it) {
    --bucketIdx;
    sBucketIndex.push_back(bucketIdx);
    BucketHolderType& bucket = *it;
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
    BucketHolderType& bucket = (*sBuckets)[planeObj.x24_targetBucket];
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

void CCubeRenderer::GenerateReflectionTex() {
  // TODO
}

void CCubeRenderer::GenerateFogVolumeRampTex() {
  constexpr double fogVolFar = 750.0;
  constexpr double fogVolNear = 0.2;
  u8* data = x1b8_fogVolumeRamp.Lock();
  u16 height = x1b8_fogVolumeRamp.GetHeight();
  u16 width = x1b8_fogVolumeRamp.GetWidth();
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      const int tmp = int(y << 16 | x << 8 | 0x7f);
      const double a =
          zeus::clamp(0.0,
                      (-150.0 / (tmp / double(0xffffff) * (fogVolFar - fogVolNear) - fogVolFar) - fogVolNear) * 3.0 /
                          (fogVolFar - fogVolNear),
                      1.0);
      data[y * width + x] = (a * a + a) / 2.0;
    }
  }
  x1b8_fogVolumeRamp.UnLock();
}

void CCubeRenderer::GenerateSphereRampTex() {
  u8* data = x220_sphereRamp.Lock();
  const size_t height = x220_sphereRamp.GetHeight();
  const size_t width = x220_sphereRamp.GetWidth();
  const float halfRes = height / 2.f;
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      const zeus::CVector2f vec{
          (static_cast<float>(x) - halfRes) / halfRes,
          (static_cast<float>(y) - halfRes) / halfRes,
      };
      data[y * width + x] = 255 - zeus::clamp(0.f, vec.canBeNormalized() ? vec.magnitude() : 0.f, 1.f) * 255;
    }
  }
  x220_sphereRamp.UnLock();
}

void CCubeRenderer::LoadThermoPalette() {
  auto* out = x288_thermoPalette.Lock();
  TToken<CTexture> token = xc_store.GetObj("TXTR_ThermoPalette");
  const auto* data = token.GetObj()->GetPalette()->GetPaletteData();
  memcpy(out, data, 32);
  x288_thermoPalette.UnLock();
}

void CCubeRenderer::ReallyDrawPhazonSuitIndirectEffect(const zeus::CColor& vertColor, CTexture& maskTex,
                                                       CTexture& indTex, const zeus::CColor& modColor, float scale,
                                                       float offX, float offY) {
  // TODO
}

void CCubeRenderer::ReallyDrawPhazonSuitEffect(const zeus::CColor& modColor, CTexture& maskTex) {
  // TODO
}

void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float blurRadius, float f2) {
  // TODO
}

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

void CCubeRenderer::DrawUnsortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) {
  SCOPED_GRAPHICS_DEBUG_GROUP(
      fmt::format(FMT_STRING("CCubeRenderer::DrawUnsortedGeometry areaIdx={} mask={} targetMask={}"), areaIdx, mask,
                  targetMask)
          .c_str(),
      zeus::skBlue);

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
  SCOPED_GRAPHICS_DEBUG_GROUP(
      fmt::format(FMT_STRING("CCubeRenderer::DrawSortedGeometry areaIdx={} mask={} targetMask={}"), areaIdx, mask,
                  targetMask)
          .c_str(),
      zeus::skBlue);

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
  SCOPED_GRAPHICS_DEBUG_GROUP(
      fmt::format(FMT_STRING("CCubeRenderer::DrawAreaGeometry areaIdx={} mask={} targetMask={}"), areaIdx, mask,
                  targetMask)
          .c_str(),
      zeus::skBlue);

  x318_30_inAreaDraw = true;
  SetupRendererStates(true);
  constexpr CModelFlags flags{0, 0, 3, zeus::skWhite};

  for (CAreaListItem& item : x1c_areaListItems) {
    if (areaIdx == -1 || item.x18_areaIdx == areaIdx) {
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

        model->SetArraysCurrent();
        for (const auto* surf = model->GetFirstUnsortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
          model->DrawSurface(*surf, flags);
        }
        for (const auto* surf = model->GetFirstSortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
          model->DrawSurface(*surf, flags);
        }
      }
    }
  }

  SetupCGraphicsState();
  x318_30_inAreaDraw = false;
}

void CCubeRenderer::RenderBucketItems(const CAreaListItem* item) {
  SCOPED_GRAPHICS_DEBUG_GROUP(
      fmt::format(FMT_STRING("CCubeRenderer::RenderBucketItems areaIdx={}"), item->x18_areaIdx).c_str(), zeus::skBlue);

  CCubeModel* lastModel = nullptr;
  EDrawableType lastDrawableType = EDrawableType::Invalid;
  for (u16 idx : Buckets::sBucketIndex) {
    BucketHolderType& bucket = (*Buckets::sBuckets)[idx];
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

void CCubeRenderer::PostRenderFogs() {
  for (const auto& warp : x2c4_spaceWarps) {
    ReallyDrawSpaceWarp(warp.first, warp.second);
  }
  x2c4_spaceWarps.clear();

  x2ac_fogVolumes.sort([](const CFogVolumeListItem& a, const CFogVolumeListItem& b) {
    zeus::CAABox aabbA = a.x34_aabb.getTransformedAABox(a.x0_transform);
    bool insideA =
        aabbA.pointInside(zeus::CVector3f(CGraphics::g_ViewPoint.x(), CGraphics::g_ViewPoint.y(), aabbA.min.z()));

    zeus::CAABox aabbB = b.x34_aabb.getTransformedAABox(b.x0_transform);
    bool insideB =
        aabbB.pointInside(zeus::CVector3f(CGraphics::g_ViewPoint.x(), CGraphics::g_ViewPoint.y(), aabbB.min.z()));

    if (insideA != insideB) {
      return insideA;
    }

    float dotA = aabbA.furthestPointAlongVector(CGraphics::g_ViewMatrix.basis[1]).dot(CGraphics::g_ViewMatrix.basis[1]);
    float dotB = aabbB.furthestPointAlongVector(CGraphics::g_ViewMatrix.basis[1]).dot(CGraphics::g_ViewMatrix.basis[1]);
    return dotA < dotB;
  });
  for (const CFogVolumeListItem& fog : x2ac_fogVolumes) {
    CGraphics::SetModelMatrix(fog.x0_transform);
    ReallyRenderFogVolume(fog.x30_color, fog.x34_aabb, fog.x4c_model.GetObj(), fog.x5c_skinnedModel);
  }
  x2ac_fogVolumes.clear();
}

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

constexpr bool TestBit(const u32* words, size_t bit) { return (words[bit / 32] & (1U << (bit & 0x1f))) != 0; }

void CCubeRenderer::ActivateLightsForModel(const CAreaListItem* areaItem, CCubeModel& model) {
  constexpr u32 LightCount = 4;
  GX::LightMask lightMask;

  if (!x300_dynamicLights.empty()) {
    std::array<u32, LightCount> addedLights{};
    std::array<float, LightCount> lightRads{-1.f, -1.f, -1.f, -1.f};

    u32 lightOctreeWordCount = 0;
    const u32* lightOctreeWords = nullptr;
    if (areaItem != nullptr && model.GetIndex() != UINT32_MAX) {
      lightOctreeWordCount = areaItem->x4_octTree->x14_bitmapWordCount;
      lightOctreeWords = areaItem->x1c_lightOctreeWords.data();
    }

    u32 lightIdx = 0;
    for (const auto& light : x300_dynamicLights) {
      if (lightIdx >= LightCount) {
        break;
      }

      if (lightOctreeWords == nullptr || TestBit(lightOctreeWords, model.GetIndex())) {
        bool loaded = false;
        const float radius =
            model.GetBounds().intersectionRadius(zeus::CSphere(light.GetPosition(), light.GetRadius()));

        if (lightIdx > 0) {
          for (u32 i = 0; i < lightIdx; ++i) {
            if (addedLights[i] == light.GetId()) {
              if (radius >= 0.f && radius < lightRads[i]) {
                lightRads[i] = radius;
                CGraphics::LoadLight(i, light);
                loaded = true;
              }
              break;
            }
          }
        }

        if (!loaded) {
          lightRads[lightIdx] = radius;
          if (radius >= 0.f) {
            CGraphics::LoadLight(lightIdx, light);
            addedLights[lightIdx] = light.GetId();
            lightMask.set(lightIdx);
            ++lightIdx;
          }
        }
      }

      lightOctreeWords += lightOctreeWordCount;
    }
  }

  if (lightMask.any()) {
    CGraphics::SetLightState(lightMask);
    CGX::SetChanMatColor(CGX::EChannelId::Channel0, zeus::skWhite);
  } else {
    CGraphics::DisableAllLights();
    CGX::SetChanMatColor(CGX::EChannelId::Channel0, CGX::GetChanAmbColor(CGX::EChannelId::Channel0));
  }
}

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
  auto left = static_cast<float>(centered ? CGraphics::GetViewportLeft() - CGraphics::GetViewportHalfWidth()
                                          : CGraphics::GetViewportLeft());
  auto bottom = static_cast<float>(centered ? CGraphics::GetViewportTop() - CGraphics::GetViewportHalfHeight()
                                            : CGraphics::GetViewportTop());
  auto right = static_cast<float>(CGraphics::GetViewportLeft() +
                                  (centered ? CGraphics::GetViewportWidth() / 2 : CGraphics::GetViewportWidth()));
  auto top = static_cast<float>(CGraphics::GetViewportTop() +
                                (centered ? CGraphics::GetViewportHeight() / 2 : CGraphics::GetViewportHeight()));
  CGraphics::SetOrtho(left, right, top, bottom, znear, zfar);
  CGraphics::SetViewPointMatrix({});
  CGraphics::SetModelMatrix({});
  return {{left, bottom}, {right, top}};
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

  x318_27_currentRGBA6 = x318_26_requestRGBA6;
  if (!x318_31_persistRGBA6) {
    x318_26_requestRGBA6 = false;
  }

  // GXSetPixelFmt(x318_27_currentRGBA6, GX_ZC_LINEAR);
  GXSetAlphaUpdate(true);
  GXSetDstAlpha(true, 0.f);
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
    if (xc8_pvs) {
      xc8_pvs->SetState(EPVSVisSetState(value));
    }
  } else if (option == EDebugOption::PVSMode) {
    xc0_pvsMode = EPVSMode(value);
  } else if (option == EDebugOption::FogDisabled) {
    x318_28_disableFog = true;
  }
}

void CCubeRenderer::BeginPrimitive(IRenderer::EPrimitiveType type, s32 nverts) {
  constexpr std::array vtxDescList{
      GXVtxDescList{GX_VA_POS, GX_DIRECT},
      GXVtxDescList{GX_VA_NRM, GX_DIRECT},
      GXVtxDescList{GX_VA_CLR0, GX_DIRECT},
      GXVtxDescList{GX_VA_NULL, GX_NONE},
  };
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, {}, GX_DF_NONE, GX_AF_NONE);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  x18_primVertCount = nverts;
  CGX::SetVtxDescv(vtxDescList.data());
  CGX::Begin(GXPrimitive(type), GX_VTXFMT0, nverts);
}

void CCubeRenderer::BeginLines(s32 nverts) { BeginPrimitive(EPrimitiveType::Lines, nverts); }

void CCubeRenderer::BeginLineStrip(s32 nverts) { BeginPrimitive(EPrimitiveType::LineStrip, nverts); }

void CCubeRenderer::BeginTriangles(s32 nverts) { BeginPrimitive(EPrimitiveType::Triangles, nverts); }

void CCubeRenderer::BeginTriangleStrip(s32 nverts) { BeginPrimitive(EPrimitiveType::TriangleStrip, nverts); }

void CCubeRenderer::BeginTriangleFan(s32 nverts) { BeginPrimitive(EPrimitiveType::TriangleFan, nverts); }

void CCubeRenderer::PrimVertex(const zeus::CVector3f& vertex) {
  --x18_primVertCount;
  GXPosition3f32(vertex);
  GXNormal3f32(x2e4_primNormal);
  GXColor4f32(x2e0_primColor);
}

void CCubeRenderer::PrimNormal(const zeus::CVector3f& normal) { x2e4_primNormal = normal; }

void CCubeRenderer::PrimColor(float r, float g, float b, float a) { PrimColor({r, g, b, a}); }

void CCubeRenderer::PrimColor(const zeus::CColor& color) { x2e0_primColor = color; }

void CCubeRenderer::EndPrimitive() {
  while (x18_primVertCount > 0) {
    PrimVertex(zeus::skZero3f);
  }
  CGX::End();
}

void CCubeRenderer::SetAmbientColor(const zeus::CColor& color) { CGraphics::SetAmbientColor(color); }

void CCubeRenderer::DrawString(const char* string, s32 x, s32 y) { x10_font.DrawString(string, x, y, zeus::skWhite); }

u32 CCubeRenderer::GetFPS() { return CGraphics::GetFPS(); }

void CCubeRenderer::CacheReflection(IRenderer::TReflectionCallback cb, void* ctx, bool clearAfter) {
  // TODO
}

void CCubeRenderer::DrawSpaceWarp(const zeus::CVector3f& pt, float strength) {
  if (pt.z() < 1.f) {
    ReallyDrawSpaceWarp(pt, strength);
  }
}

void CCubeRenderer::DrawThermalModel(CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                                     TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) {
  model.UpdateLastFrame();
  DoThermalModelDraw(model.GetInstance(), multCol, addCol, positions, normals, flags);
}

void CCubeRenderer::DrawModelDisintegrate(CModel& model, CTexture& tex, const zeus::CColor& color,
                                          TConstVectorRef positions, TConstVectorRef normals, float t) {
  tex.Load(GX_TEXMAP0, EClampMode::Clamp);
  CGX::SetNumIndStages(0);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(2);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
  CGX::SetTevKColor(GX_KCOLOR0, color);
  const auto bounds = model.GetInstance().GetBounds();
  const auto rotation = zeus::CTransform::RotateX(zeus::degToRad(-45.f));
  const auto transformedBounds = bounds.getTransformedAABox(rotation);
  const auto xf = zeus::CTransform::Scale(5.f / (transformedBounds.max - transformedBounds.min)) *
                  zeus::CTransform::Translate(-transformedBounds.min) * rotation;
  const zeus::CTransform ptTex1{
      zeus::CMatrix3f{
          1.f, 1.f, 0.f,
          0.f, 0.f, 1.f,
          0.f, 0.f, 0.f,
      },
      zeus::CVector3f{t * -0.85f - 0.15f, -(1.f - t) * 6.f + 1.f, 1.f},
  };
  const zeus::CTransform ptTex0{
      zeus::CMatrix3f{
          1.f, 1.f, 0.f,
          0.f, 0.f, 1.f,
          0.f, 0.f, 0.f,
      },
      zeus::CVector3f{t, ptTex1.origin.y(), 1.f},
  };
  GXLoadTexMtxImm(&xf, GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(&ptTex0, GX_PTTEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(&ptTex1, GX_PTTEXMTX1, GX_MTX3x4);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX1);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, true);
  model.UpdateLastFrame();
  model.GetInstance().DrawFlat(positions, normals, ESurfaceSelection::All);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void CCubeRenderer::DrawModelFlat(CModel& model, const CModelFlags& flags, bool unsortedOnly, TConstVectorRef positions,
                                  TConstVectorRef normals) {
  if (flags.x0_blendMode >= 7) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else if (flags.x0_blendMode >= 5) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }
  CGX::SetZMode(true, flags.x2_flags & CModelFlagBits::DepthTest ? GX_LEQUAL : GX_ALWAYS,
                flags.x2_flags.IsSet(CModelFlagBits::DepthUpdate));
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(0);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKColor(GX_KCOLOR0, flags.x4_color);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);
  model.UpdateLastFrame();
  model.GetInstance().DrawFlat(positions, normals, unsortedOnly ? ESurfaceSelection::Unsorted : ESurfaceSelection::All);
}

void CCubeRenderer::SetWireframeFlags(s32 flags) {
  CCubeModel::SetModelWireframe((flags & 0x1) != 0);
  x318_25_drawWireframe = (flags & 0x2) != 0;
}

void CCubeRenderer::SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) {
  CGraphics::SetFog(x318_28_disableFog ? ERglFogMode::None : mode, startz, endz, color);
}

void CCubeRenderer::RenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb,
                                    const TLockedToken<CModel>* model, const CSkinnedModel* sModel) {
  if (!x318_28_disableFog) {
    x2ac_fogVolumes.emplace_back(CGraphics::g_GXModelMatrix, color, aabb, model, sModel);
  }
}

void CCubeRenderer::SetThermal(bool thermal, float level, const zeus::CColor& color) {
  x318_29_thermalVisor = thermal;
  x2f0_thermalVisorLevel = level;
  x2f4_thermColor = color;
  CDecal::SetMoveRedToAlphaBuffer(false);
  CElementGen::SetMoveRedToAlphaBuffer(false);
}

void CCubeRenderer::SetThermalColdScale(float scale) { x2f8_thermColdScale = zeus::clamp(0.f, scale, 1.f); }

void CCubeRenderer::DoThermalBlendCold() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::DoThermalBlendCold", zeus::skBlue);

  // Capture EFB
  x318_26_requestRGBA6 = true;
  GXSetAlphaUpdate(true);
  GXSetDstAlpha(false, 0);
  const auto height = CGraphics::GetViewportHeight();
  const auto width = CGraphics::GetViewportWidth();
  const auto top = CGraphics::GetViewportTop();
  const auto left = CGraphics::GetViewportLeft();
  CGX::SetZMode(true, GX_LEQUAL, false);
  GXSetTexCopySrc(left, top, width, height);
  GXSetTexCopyDst(width, height, GX_TF_I4, false);
  GXCopyTex(CGraphics::sSpareTextureData, true);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_I4, nullptr, GX_TEXMAP7);

  // Upload random static texture (game reads from .text)
  const u8* buf = CDvdFile::GetDolBuf() + 0x4f60;
  u8* out = m_thermalRandomStatic.Lock();
  memcpy(out, buf + ROUND_UP_32(x2a8_thermalRand.Next()), m_thermalRandomStatic.GetMemoryAllocated());
  m_thermalRandomStatic.UnLock();
  m_thermalRandomStatic.Load(GX_TEXMAP0, EClampMode::Clamp);
  m_thermalRandomStatic.Load(GX_TEXMAP1, EClampMode::Clamp);

  // Configure indirect texturing
  const float level = std::clamp(x2f0_thermalVisorLevel * 0.5f, 0.f, 0.5f);
  const aurora::Mat3x2<float> mtx{
      aurora::Vec2{(1.f - level) * 0.1f, 0.f},
      aurora::Vec2{0.f, 0.f},
      aurora::Vec2{0.f, level},
  };
  GXSetIndTexMtx(GX_ITM_0, &mtx, -2);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, false,
                      false, GX_ITBA_OFF);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);

  // Configure register colors
  const auto color0 = zeus::CColor::lerp(x2f4_thermColor, zeus::skWhite, x2f8_thermColdScale);
  const float bAlpha = x2f8_thermColdScale < 0.5f ? x2f8_thermColdScale * 2.f : 1.f;
  const float bFac = (1.f - bAlpha) / 8.f;
  const zeus::CColor color1{bFac, bAlpha};
  float cFac;
  if (x2f8_thermColdScale < 0.25f) {
    cFac = 0.f;
  } else if (x2f8_thermColdScale >= 1.f) {
    cFac = 1.f;
  } else {
    cFac = (x2f8_thermColdScale - 0.25f) * 4.f / 3.f;
  }
  const zeus::CColor color2{cFac, cFac};
  GXSetTevColor(GX_TEVREG0, to_gx_color(color0));
  GXSetTevColor(GX_TEVREG1, to_gx_color(color1));
  GXSetTevColor(GX_TEVREG2, to_gx_color(color2));

  // Configure TEV stage 0
  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_C2);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_A2);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR_NULL);

  // Configure TEV stage 1
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);

  // Configure everything else
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(1);
  CGX::SetZMode(false, GX_ALWAYS, false);
  constexpr std::array vtxDescList{
      GXVtxDescList{GX_VA_POS, GX_DIRECT},
      GXVtxDescList{GX_VA_TEX0, GX_DIRECT},
      GXVtxDescList{GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescList.data());
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

  // Backup & set viewport/projection
  const auto backupViewMatrix = CGraphics::g_ViewMatrix;
  const auto backupProjectionState = CGraphics::GetProjectionState();
  CGraphics::SetOrtho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix({});
  CGraphics::SetModelMatrix({});
  GXPixModeSync();

  // Draw
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.5f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.5f, static_cast<float>(height));
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(static_cast<float>(width), 0.5f, static_cast<float>(height));
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(static_cast<float>(width), 0.5f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  // Cleanup
  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  GXSetDstAlpha(false, 255);
  CGraphics::SetProjectionState(backupProjectionState);
  CGraphics::SetViewPointMatrix(backupViewMatrix);
  CDecal::SetMoveRedToAlphaBuffer(true);
  CElementGen::SetMoveRedToAlphaBuffer(true);
}

void CCubeRenderer::DoThermalBlendHot() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::DoThermalBlendHot", zeus::skRed);

  GXSetAlphaUpdate(false);
  GXSetDstAlpha(true, 0);
  const auto height = CGraphics::GetViewportHeight();
  const auto width = CGraphics::GetViewportWidth();
  const auto top = CGraphics::GetViewportTop();
  const auto left = CGraphics::GetViewportLeft();
  CGX::SetZMode(true, GX_LEQUAL, true);
  GXSetTexCopySrc(left, top, width, height);
  GXSetTexCopyDst(width, height, GX_TF_I4, false);
  GXCopyTex(CGraphics::sSpareTextureData, false);
  x288_thermoPalette.Load();
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_C4, GX_TLUT0, nullptr, GX_TEXMAP7);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetZMode(false, GX_LEQUAL, false);
  constexpr std::array vtxDescList{
      GXVtxDescList{GX_VA_POS, GX_DIRECT},
      GXVtxDescList{GX_VA_TEX0, GX_DIRECT},
      GXVtxDescList{GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescList.data());
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_CLEAR);

  // Backup & set viewport/projection
  const auto backupViewMatrix = CGraphics::g_ViewMatrix;
  const auto backupProjectionState = CGraphics::GetProjectionState();
  CGraphics::SetOrtho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix({});
  CGraphics::SetModelMatrix({});
  GXPixModeSync();

  // Draw
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.5f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.5f, static_cast<float>(height));
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(static_cast<float>(width), 0.5f, static_cast<float>(height));
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(static_cast<float>(width), 0.5f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  // Cleanup
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  GXSetAlphaUpdate(true);
  CGraphics::SetProjectionState(backupProjectionState);
  CGraphics::SetViewPointMatrix(backupViewMatrix);
  CDecal::SetMoveRedToAlphaBuffer(false);
  CElementGen::SetMoveRedToAlphaBuffer(false);
}

u32 CCubeRenderer::GetStaticWorldDataSize() {
  // TODO
  return 0;
}

void CCubeRenderer::SetGXRegister1Color(const zeus::CColor& color) { GXSetTevColor(GX_TEVREG1, to_gx_color(color)); }

void CCubeRenderer::SetWorldLightFadeLevel(float level) { x2fc_tevReg1Color = zeus::CColor(level, level, level, 1.f); }

void CCubeRenderer::PrepareDynamicLights(const std::vector<CLight>& lights) {
  x300_dynamicLights = lights;

  for (CAreaListItem& area : x1c_areaListItems) {
    if (const CAreaRenderOctTree* arot = area.x4_octTree) {
      area.x1c_lightOctreeWords.clear();
      area.x1c_lightOctreeWords.resize(arot->x14_bitmapWordCount * lights.size());
      u32* wordPtr = area.x1c_lightOctreeWords.data();
      for (const CLight& light : lights) {
        float radius = light.GetRadius();
        zeus::CVector3f vMin = light.GetPosition() - radius;
        zeus::CVector3f vMax = light.GetPosition() + radius;
        zeus::CAABox aabb(vMin, vMax);
        arot->FindOverlappingModels(wordPtr, aabb);
        wordPtr += arot->x14_bitmapWordCount;
      }
    }
  }
}

void CCubeRenderer::AllocatePhazonSuitMaskTexture() {
  x318_26_requestRGBA6 = true;
  if (!x314_phazonSuitMask) {
    x314_phazonSuitMask = std::make_unique<CTexture>(ETexelFormat::I8, CGraphics::GetViewportWidth() / 4,
                                                     CGraphics::GetViewportHeight() / 4, 1, "Phazon Suit Mask"sv);
  }
  x310_phazonSuitMaskCountdown = 2;
}

void CCubeRenderer::DrawPhazonSuitIndirectEffect(const zeus::CColor& nonIndirectMod,
                                                 const TLockedToken<CTexture>& indTex, const zeus::CColor& indirectMod,
                                                 float blurRadius, float scale, float offX, float offY) {
  if (x318_27_currentRGBA6 && x310_phazonSuitMaskCountdown != 0) {
    const auto backupViewMatrix = CGraphics::g_ViewMatrix;
    const auto backupProjectionState = CGraphics::GetProjectionState();
    if (!x314_phazonSuitMask || x314_phazonSuitMask->GetWidth() != CGraphics::GetViewportWidth() / 4 ||
        x314_phazonSuitMask->GetHeight() != CGraphics::GetViewportHeight() / 4) {
      return;
    }
    DoPhazonSuitIndirectAlphaBlur(blurRadius, blurRadius);
    // TODO copy into x314_phazonSuitMask
    if (indTex) {
      ReallyDrawPhazonSuitIndirectEffect(zeus::skWhite, *x314_phazonSuitMask, const_cast<CTexture&>(*indTex),
                                         indirectMod, scale, offX, offY);
    } else {
      ReallyDrawPhazonSuitEffect(nonIndirectMod, *x314_phazonSuitMask);
    }
    // TODO unlock x314
    CGraphics::SetViewPointMatrix(backupViewMatrix);
    CGraphics::SetProjectionState(backupProjectionState);
    x310_phazonSuitMaskCountdown = 2;
  }
  GXSetDstAlpha(false, 0.f);
}

void CCubeRenderer::DrawXRayOutline(const zeus::CAABox& aabb) {
  // TODO
}

std::list<CCubeRenderer::CAreaListItem>::iterator
CCubeRenderer::FindStaticGeometry(const std::vector<CMetroidModelInstance>* geometry) {
  return std::find_if(x1c_areaListItems.begin(), x1c_areaListItems.end(),
                      [&](const CAreaListItem& item) { return item.x0_geometry == geometry; });
}

void CCubeRenderer::FindOverlappingWorldModels(std::vector<u32>& modelBits, const zeus::CAABox& aabb) const {
  u32 bitmapWords = 0;
  for (const CAreaListItem& item : x1c_areaListItems) {
    if (item.x4_octTree != nullptr) {
      bitmapWords += item.x4_octTree->x14_bitmapWordCount;
    }
  }

  if (bitmapWords == 0u) {
    modelBits.clear();
    return;
  }

  modelBits.clear();
  modelBits.resize(bitmapWords);

  u32 curWord = 0;
  for (const CAreaListItem& item : x1c_areaListItems) {
    if (item.x4_octTree == nullptr) {
      continue;
    }

    item.x4_octTree->FindOverlappingModels(modelBits.data() + curWord, aabb);

    u32 wordModel = 0;
    for (u32 i = 0; i < item.x4_octTree->x14_bitmapWordCount; ++i, wordModel += 32) {
      u32& word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }
      for (u32 j = 0; j < 32; ++j) {
        if (((1U << j) & word) != 0) {
          const zeus::CAABox& modelAABB = (*item.x10_models)[wordModel + j]->GetBounds();
          if (!modelAABB.intersects(aabb)) {
            word &= ~(1U << j);
          }
        }
      }
    }

    curWord += item.x4_octTree->x14_bitmapWordCount;
  }
}

s32 CCubeRenderer::DrawOverlappingWorldModelIDs(s32 alphaVal, const std::vector<u32>& modelBits,
                                                const zeus::CAABox& aabb) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::DrawOverlappingWorldModelIDs", zeus::skGrey);
  SetupRendererStates(true);

  constexpr CModelFlags flags{0, 0, 3, zeus::skWhite};

  u32 curWord = 0;
  for (const CAreaListItem& item : x1c_areaListItems) {
    if (item.x4_octTree == nullptr) {
      continue;
    }

    u32 wordModel = 0;
    for (u32 i = 0; i < item.x4_octTree->x14_bitmapWordCount; ++i, wordModel += 32) {
      const u32& word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }
      for (u32 j = 0; j < 32; ++j) {
        if (((1U << j) & word) != 0) {
          if (alphaVal > 255) {
            SetupCGraphicsState();
            return alphaVal;
          }

          auto& model = *(*item.x10_models)[wordModel + j];
          GXSetDstAlpha(true, alphaVal);
          CCubeMaterial::KillCachedViewDepState();
          model.SetArraysCurrent();
          for (const auto* surf = model.GetFirstUnsortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
            if (surf->GetBounds().intersects(aabb)) {
              model.DrawSurface(*surf, flags);
            }
          }
          alphaVal += 4;
        }
      }
    }

    curWord += item.x4_octTree->x14_bitmapWordCount;
  }

  SetupCGraphicsState();
  return alphaVal;
}

void CCubeRenderer::DrawOverlappingWorldModelShadows(s32 alphaVal, const std::vector<u32>& modelBits,
                                                     const zeus::CAABox& aabb) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CBooRenderer::DrawOverlappingWorldModelShadows", zeus::skGrey);

  u32 curWord = 0;
  for (const CAreaListItem& item : x1c_areaListItems) {
    if (item.x4_octTree == nullptr) {
      continue;
    }

    u32 wordModel = 0;
    for (u32 i = 0; i < item.x4_octTree->x14_bitmapWordCount; ++i, wordModel += 32) {
      const u32& word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }
      for (u32 j = 0; j < 32; ++j) {
        if (((1U << j) & word) != 0) {
          if (alphaVal > 255) {
            return;
          }

          auto& model = *(*item.x10_models)[wordModel + j];
          CGX::SetTevKColor(GX_KCOLOR0, zeus::CColor{0.f, static_cast<float>(alphaVal) / 255.f});
          model.SetArraysCurrent();
          for (const auto* surf = model.GetFirstUnsortedSurface(); surf != nullptr; surf = surf->GetNextSurface()) {
            if (surf->GetBounds().intersects(aabb)) {
              const auto& material = model.GetMaterialByIndex(surf->GetMaterialIndex());
              CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
              CGX::CallDisplayList(surf->GetDisplayList(), surf->GetDisplayListSize());
            }
          }
          alphaVal += 4;
        }
      }
    }

    curWord += item.x4_octTree->x14_bitmapWordCount;
  }
}

void CCubeRenderer::SetupCGraphicsState() {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix({});
  CTevCombiners::ResetStates();
  CGraphics::SetAmbientColor({0.4f});
  CGX::SetChanMatColor(CGX::EChannelId::Channel0, zeus::skWhite);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
  CGX::SetChanCtrl(CGX::EChannelId::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CCubeMaterial::EnsureTevsDirect();
}

void CCubeRenderer::SetupRendererStates(bool depthWrite) {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix({});
  CGraphics::SetAmbientColor(zeus::skClear);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, depthWrite);
  CCubeMaterial::ResetCachedMaterials();
  GXSetTevColor(GX_TEVREG1, to_gx_color(x2fc_tevReg1Color));
}

constexpr zeus::CTransform MvPostXf{
    {zeus::CVector3f{0.5f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.5f, 0.f}},
    {0.5f, 0.5f, 1.f},
};

void CCubeRenderer::DoThermalModelDraw(CCubeModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                                       TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::DoThermalModelDraw", zeus::skBlue);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, true, GX_PTTEXMTX0);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  x220_sphereRamp.Load(GX_TEXMAP0, EClampMode::Clamp);
  zeus::CTransform xf = CGraphics::g_ViewMatrix.inverse().multiplyIgnoreTranslation(CGraphics::g_GXModelMatrix);
  xf.origin.zeroOut();
  GXLoadTexMtxImm(&xf, GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(&MvPostXf, GX_PTTEXMTX0, GX_MTX3x4);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_KONST);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetTevKColor(GX_KCOLOR0, addCol);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  GXSetTevColor(GX_TEVREG0, to_gx_color(multCol));
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetZMode(flags.x2_flags.IsSet(CModelFlagBits::DepthTest), GX_LEQUAL,
                flags.x2_flags.IsSet(CModelFlagBits::DepthUpdate));
  model.DrawFlat(positions, normals,
                 flags.x2_flags.IsSet(CModelFlagBits::ThermalUnsortedOnly) ? ESurfaceSelection::Unsorted
                                                                           : ESurfaceSelection::All);
}

void CCubeRenderer::ReallyDrawSpaceWarp(const zeus::CVector3f& pt, float strength) {
  return; // TODO

  float vpLeft = static_cast<float>(CGraphics::GetViewportLeft());
  float vpTop = static_cast<float>(CGraphics::GetViewportTop());
  float vpHalfWidth = static_cast<float>(CGraphics::GetViewportWidth() / 2);
  float vpHalfHeight = static_cast<float>(CGraphics::GetViewportHeight() / 2);
  float local_100 = vpLeft + vpHalfWidth * pt.x() + vpHalfWidth;
  float local_fc = vpTop + vpHalfHeight * -pt.y() + vpHalfHeight;
  zeus::CVector2i CStack264{static_cast<s32>(local_100) & ~3, static_cast<s32>(local_fc) & ~3};
  auto v2right = CStack264 - zeus::CVector2i{96, 96};
  auto v2left = CStack264 + zeus::CVector2i{96, 96};
  zeus::CVector2f uv1min{0.f, 0.f};
  zeus::CVector2f uv1max{1.f, 1.f};

  s32 aleft = CGraphics::GetViewportLeft() & ~3;
  s32 atop = CGraphics::GetViewportTop() & ~3;
  s32 aright = (CGraphics::GetViewportLeft() + CGraphics::GetViewportWidth() + 3) & ~3;
  s32 abottom = (CGraphics::GetViewportTop() + CGraphics::GetViewportHeight() + 3) & ~3;
  if (v2right.x < aleft) {
    uv1min.x() = static_cast<float>(aleft - v2right.x) * 0.005208333f;
    v2right.x = aleft;
  }
  if (v2right.y < atop) {
    uv1min.y() = static_cast<float>(atop - v2right.x) * 0.005208333f;
    v2right.y = atop;
  }
  if (v2left.x > aright) {
    uv1max.x() = 1.f - static_cast<float>(v2left.x - aright) * 0.005208333f;
    v2left.x = aright;
  }
  if (v2left.y > abottom) {
    uv1max.y() = 1.f - static_cast<float>(v2left.y - abottom) * 0.005208333f;
    v2left.y = abottom;
  }
  const auto v2sub = v2left - v2right;
  if (v2sub.x > 0 && v2sub.y > 0) {
    GXFogType fogType;
    float fogStartZ;
    float fogEndZ;
    float fogNearZ;
    float fogFarZ;
    GXColor fogColor;
    CGX::GetFog(&fogType, &fogStartZ, &fogEndZ, &fogNearZ, &fogFarZ, &fogColor);
    CGX::SetFog(GX_FOG_NONE, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);
    GXSetTexCopySrc(v2right.x, v2right.y, v2sub.x, v2sub.y);
    GXSetTexCopyDst(v2sub.x, v2sub.y, GX_TF_RGBA8, false);
    GXCopyTex(CGraphics::sSpareTextureData, false);
    GXPixModeSync();
    CGraphics::LoadDolphinSpareTexture(v2sub.x, v2sub.y, GX_TF_RGBA8, nullptr, GX_TEXMAP7);
    x150_reflectionTex.Load(GX_TEXMAP1, EClampMode::Clamp);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR_NULL);
  }
}

void CCubeRenderer::ReallyRenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const CModel* model,
                                          const CSkinnedModel* sModel) {
  // TODO
}
} // namespace metaforce
