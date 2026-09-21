#include "MetaRender/CCubeRenderer.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDrawable.hpp"
#include "Kyoto/Graphics/CDrawablePlaneObject.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CLight.hpp" // IWYU pragma: keep
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "Weapons/CDecal.hpp"
#include "Weapons/IWeaponRenderer.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "dolphin/types.h"
#include "rstl/algorithm.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#include <string.h>
#include <type_traits>

CCubeRenderer* CCubeRenderer::sRenderer = nullptr;
static CModelFlags skNormalFlag = CModelFlags::Normal();
static CModelFlags skNormalFlagNoUpdate = CModelFlags::Normal().DepthCompareUpdate(true, false);

namespace Buckets {
typedef rstl::reserved_vector< CDrawable*, 128 > BucketHolderType;

static rstl::pair< float, float > skWorstMinMaxDistance(99999.f, -99999.f);
static rstl::reserved_vector< ushort, 50 > sBucketIndex;
static rstl::reserved_vector< CDrawable, 512 >* sData;
static rstl::reserved_vector< BucketHolderType, 50 >* sBuckets;
static rstl::pair< float, float > sMinMaxDistance = skWorstMinMaxDistance;
static rstl::reserved_vector< CDrawablePlaneObject, 8 >* sPlaneObjectData;
static rstl::reserved_vector< ushort, 8 >* sPlaneObjectBucket;

static inline const rstl::reserved_vector< ushort, 50 >& GetBucketData() { return sBucketIndex; }
static inline const rstl::reserved_vector< BucketHolderType, 50 >& GetBucketHolder() {
  return *sBuckets;
}

void Init() {
#define HOLDER(var)                                                                                \
  static uchar var##Holder[sizeof(__typeof__(*var)) + 4];                                          \
  var = new (var##Holder) std::type_identity< __typeof__(*var) >::type();

  HOLDER(sData);
  HOLDER(sBuckets);
  HOLDER(sPlaneObjectData);
  HOLDER(sPlaneObjectBucket);

  sBuckets->resize(50);
  sMinMaxDistance = skWorstMinMaxDistance;
}

void Shutdown() {
  sData = nullptr;
  sBuckets = nullptr;
  sPlaneObjectData = nullptr;
  sPlaneObjectBucket = nullptr;
}

void Insert(const CVector3f& pos, const CAABox& aabb, EDrawableType dtype, const void* data,
            const CPlane& plane, const ushort extraSort) {
  AUTO_REF(tmpData, *sData);
  if (tmpData.size() == tmpData.capacity()) {
    return;
  }
  float dist = CVector3f::Dot(pos, plane.GetNormal()) - plane.GetConstant();
  const CDrawable& drawable = CDrawable(dtype, extraSort, dist, aabb, data);
  tmpData.push_back(drawable);
  sMinMaxDistance.first = rstl::min_val(dist, sMinMaxDistance.first);
  sMinMaxDistance.second = rstl::max_val(dist, sMinMaxDistance.second);
#ifdef __MWERKS__
  __dcbt(&tmpData.back() + 1, 0);
#endif
}

void InsertPlaneObject(float closeDist, float farDist, const CAABox& aabb, bool invertTest,
                       const CPlane& plane, bool zOnly, EDrawableType dtype, const void* data) {
  AUTO_REF(planeObjects, *sPlaneObjectData);
  if (planeObjects.size() == planeObjects.capacity()) {
    return;
  }
  planeObjects.push_back(
      CDrawablePlaneObject(dtype, closeDist, farDist, aabb, invertTest, plane, zOnly, data));
}

struct planeSorter {
  bool operator()(const ushort a, const ushort b) const {
    const CDrawablePlaneObject& planeA = (*sPlaneObjectData)[a];
    const CDrawablePlaneObject& planeB = (*sPlaneObjectData)[b];
    return planeA.GetDistance() < planeB.GetDistance();
  }
};

struct sorter {
  bool operator()(CDrawable* a, CDrawable* b) const {
    const float distA = a->GetDistance();
    const float distB = b->GetDistance();
    if (distA == distB) {
      return a->GetExtraSort() > b->GetExtraSort();
    }
    return distA > distB;
  }
};

void Sort() {
  const float negMin = -sMinMaxDistance.first;
  const float delta = rstl::max_val(sMinMaxDistance.second - sMinMaxDistance.first, 1.f);
  float pitch = 1.f / (delta * (1.f / 49.f));

  int idx = 0;
  for (CDrawablePlaneObject* it = sPlaneObjectData->begin(); it != sPlaneObjectData->end();
       ++it, ++idx) {
    if (sPlaneObjectBucket->size() < sPlaneObjectBucket->capacity()) {
      sPlaneObjectBucket->push_back(static_cast< ushort >(idx));
    }
  }

  int precision = 50;
  AUTO_REF(planeBuckets, *sPlaneObjectBucket);
  if (planeBuckets.size() != 0) {
    rstl::sort(planeBuckets.begin(), planeBuckets.end(), planeSorter());

    precision = 50 / (planeBuckets.size() + 1);
    pitch = 1.f / (delta * (1.f / static_cast< float >(precision - 2)));

    short accum = 0;
    for (AUTO(it, planeBuckets.begin()); it != planeBuckets.end(); ++it, ++accum) {
      (*sPlaneObjectData)[*it].SetBucketIndex(precision * (accum + 1));
    }
  }

  AUTO_REF(planeBucketData, *sPlaneObjectBucket);
  AUTO_REF(planeObjectData, *sPlaneObjectData);
  for (AUTO(it, sData->begin()); it != sData->end(); ++it) {
    int slot = -1;
    const float relDist = negMin + it->GetDistance();

    if (planeBucketData.size() == 0) {
      slot = rstl::max_val(rstl::min_val(49, static_cast< int >(relDist * pitch)), 1);
    } else {
      slot = rstl::max_val(rstl::min_val(precision - 2, static_cast< int >(relDist * pitch)), 0);

      for (AUTO(jt, planeBucketData.begin()); jt != planeBucketData.end(); ++jt) {
        CDrawablePlaneObject& planeObj = planeObjectData[*jt];

        bool partial;
        bool full;
        if (planeObj.IsOptimalPlane()) {
          partial = it->GetBounds().GetMaxPoint().GetZ() > planeObj.GetPlane().GetConstant()
                        ? true
                        : false;
          full = it->GetBounds().GetMinPoint().GetZ() > planeObj.GetPlane().GetConstant() ? true
                                                                                          : false;
        } else {
          partial = planeObj.GetPlane().GetHeight(it->GetBounds().ClosestPointAlongVector(
                        planeObj.GetPlane().GetNormal())) > 0.f
                        ? true
                        : false;
          full = planeObj.GetPlane().GetHeight(it->GetBounds().FurthestPointAlongVector(
                     planeObj.GetPlane().GetNormal())) > 0.f
                     ? true
                     : false;
        }

        bool cont;
        if (it->GetType() == kDT_Particle) {
          cont = planeObj.IsViewInFront() ? !partial : full;
        } else if (planeObj.IsViewInFront()) {
          cont = !(partial && full);
        } else {
          cont = partial || full;
        }

        if (!cont) {
          break;
        }

        slot += precision;
      }
    }

    if (slot == -1) {
      slot = 49;
    }

    BucketHolderType& bucket = (*sBuckets)[slot];
    if (bucket.size() < bucket.capacity()) {
      bucket.push_back(it);
    }
  }

  for (int i = sBuckets->size() - 1; i >= 0; --i) {
    BucketHolderType& bucket = (*sBuckets)[i];
    sBucketIndex.push_back(static_cast< ushort >(i));
    if (bucket.size() != 0) {
      rstl::sort(bucket.begin(), bucket.end(), sorter());
    }
  }

  for (AUTO(it, planeBucketData.end() - 1); it != planeBucketData.begin() - 1; --it) {

    CDrawablePlaneObject& planeObj = planeObjectData[*it];
    BucketHolderType& bucket = (*sBuckets)[planeObj.GetBucketIndex()];
    if (bucket.size() < bucket.capacity()) {
      bucket.push_back(&planeObj);
    }
  }
}

void Clear() {
  sData->clear();
  sBucketIndex.clear();
  sPlaneObjectData->clear();
  sPlaneObjectBucket->clear();
  for (AUTO(it, sBuckets->begin()); it != sBuckets->end(); ++it) {
    it->clear();
  }
  sMinMaxDistance = skWorstMinMaxDistance;
}
} // namespace Buckets

CCubeRenderer::CAreaListItem::CAreaListItem(
    const rstl::vector< CMetroidModelInstance >* geometry, const CAreaRenderOctTree* octTree,
    const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > >& textures,
    const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > >& models, int areaIdx)
: x0_geometry(geometry)
, x4_octTree(octTree)
, x8_textures(textures)
, x10_models(models)
, x18_areaIdx(areaIdx)
, x1c_lightOctreeWords() {}

CCubeRenderer::CFogVolumeListItem::CFogVolumeListItem(const CTransform4f& xf, CColor color,
                                                      const CAABox& aabb,
                                                      const TLockedToken< CModel >* model,
                                                      const CSkinnedModel* skinnedModel)
: x0_xf(xf)
, x30_color(color)
, x34_aabb(aabb)
, x4c_model(model ? rstl::optional_object< TLockedToken< CModel > >(*model)
                  : rstl::optional_object_null())
, x5c_skinnedModel(skinnedModel) {}

CCubeRenderer::CCubeRenderer(IObjectStore& objStore, COsContext& osContext, CMemorySys& memorySys,
                             CResFactory& resFactory)
: x8_factory(resFactory)
, xc_objStore(objStore)
, x10_font(1.f)
, x18_primVertCount(0)
, x1c_areaListItems()
, x34_surfaces()
, x44_frustumPlanes(CTransform4f::Identity(), 1.5707964f, 1.f, 1.f, false, 100.f)
, xa8_drawableCallback(nullptr)
#if NONMATCHING
, xac_drawableCallbackUserData(nullptr)
#endif
, xb0_viewPlane(0.f, CUnitVector3f(CVector3f(0.f, 1.f, 0.f), CUnitVector3f::kN_Yes))
, xc0_pvsMode(0)
#if NONMATCHING
, xc4_pvsState(0)
#endif
, xc8_pvsVisSet()
, xe0_pvsAreaIdx(-1)
, xe4_blackTex(kTF_RGB565, 4, 4, 1)
, x14c_reflectionTex()
, x150_reflectionTex(kTF_IA8, 32, 32, 1)
, x1b8_fogVolumeRamp(kTF_I8, 256, 256, 1)
, x220_sphereRamp(kTF_I8, 32, 32, 1)
, x288_thermalPalette(kPF_RGB565, 16)
, x2a8_thermalRand(20)
, x2ac_fogVolumes()
, x2c4_spaceWarps()
, x2dc_reflectionAge(2)
, x2e0_primColor(CColor::White())
, x2e4_primNormal(CVector3f::Forward())
, x2f0_thermalVisorLevel(1.f)
, x2f4_thermalColor(static_cast< uchar >(255), static_cast< uchar >(0), static_cast< uchar >(255))
, x2f8_thermalColdScale(0)
, x2fc_tevReg1Color(static_cast< uchar >(255), static_cast< uchar >(0), static_cast< uchar >(255))
, x300_dynamicLights()
, x310_phazonSuitMaskCountdown(0)
, x314_phazonSuitMask()
, x318_24_reflectionDirty(false)
, x318_25_drawWireframe(false)
, x318_26_requestRGBA6(false)
, x318_27_currentRGBA6(false)
, x318_28_disableFog(false)
, x318_29_thermalVisor(false)
, x318_30_inAreaDraw(false)
, x318_31_persistRGBA6(false) {
  void* data = xe4_blackTex.Lock();
  memset(data, 0, 32);
  xe4_blackTex.UnLock();
  GenerateReflectionTex();
  GenerateFogVolumeRampTex();
  GenerateSphereRampTex();
  LoadThermoPalette();
  sRenderer = this;
  Buckets::Init();
  CSkinnedModel::AddDummySkinnedModelRef();
}

void CCubeRenderer::GenerateReflectionTex() {
  float threshold = 14.f;
  float halfScale = 128.f;

  ushort* base = static_cast< ushort* >(x150_reflectionTex.Lock());
  int texel = 0;
  for (int yBlock = 0; yBlock < 8; ++yBlock) {
    for (int xBlock = 0; xBlock < 8; ++xBlock) {
      for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
          float fx = 0.f;
          float fy = 0.f;
          CVector2f vec(static_cast< float >(xBlock * 4 + (x - 14)),
                        static_cast< float >(yBlock * 4 + (y - 14)));
          float mag = vec.Magnitude();
          if (mag <= threshold) {
            vec.Normalize();
            vec *= (threshold - mag) / threshold;
            fx = vec.GetX();
            fy = vec.GetY();
          }

          float scaledX = halfScale * fx + halfScale;
          int ix = static_cast< int >(CMath::Clamp(0.f, scaledX, 255.f));
          float scaledY = halfScale * fy + halfScale;
          int iy = static_cast< int >(CMath::Clamp(0.f, scaledY, 255.f));

          base[texel] = CBasics::SwapBytes(static_cast< ushort >((iy & 0xFF) | ((ix & 0xFF) << 8)));
          ++texel;
        }
      }
    }
  }
  x150_reflectionTex.UnLock();
}

void CCubeRenderer::GenerateFogVolumeRampTex() {
  uchar* data = static_cast< uchar* >(x1b8_fogVolumeRamp.Lock());
  memset(data, 0xFF, 0x10000);
  for (int y = 0, yOff = 0; y < 2048; ++y, yOff += 32) {
    int tileXBase = (y % 32) * 8;
    int tileYBase = (y / 32) * 4;
    uchar* ptr = &data[yOff];
    for (int x = 0; x < 32; ++x) {
      int tileX = tileXBase + (x & 7);
      int tileY = tileYBase + (x >> 3);
      uint tmp = static_cast< uint >((tileY << 16) | (tileX << 8) | 0x7F);
      double t = static_cast< double >(tmp) / static_cast< double >(0xFFFFFF);
      double a = (-(150.0 / (t * (750.f - 0.2f) - 750.0)) - 0.2f) * 3.0 / (750.f - 0.2f);
      float cf = CMath::Clamp< float >(0.f, a, 1.f);
      *ptr++ = CCast::ToUint8((0.5f * (cf * cf + cf)) * 255.f);
    }
  }
  x1b8_fogVolumeRamp.UnLock();
}

void CCubeRenderer::GenerateSphereRampTex() {
  const int height = 32;
  const int width = 32;
  const float halfRes = (height - 1) / 2.f;

  uchar* data = static_cast< uchar* >(x220_sphereRamp.Lock());
  for (int y = 0; y < height; ++y) {
    int start = y * width;
    for (int x = 0; x < width; ++x) {
      // I8 block is 8x4 (WxH)
      // Convert swizzled coords to linear
      float fx = static_cast< float >(((y % 4) << 3) + (x & 7));
      float fy = static_cast< float >(((y / 4) << 2) + (x >> 3));
      fx = (fx / halfRes) - 1.f;
      fy = (fy / halfRes) - 1.f;
      float mag = CMath::SqrtF(fx * fx + fy * fy);
      float value = CMath::Clamp(0.f, 1.f - (mag * mag), 1.f);
      data[start + x] = static_cast< uchar >(value * 255.f);
    }
  }
  x220_sphereRamp.UnLock();
}

void CCubeRenderer::LoadThermoPalette() {
  x288_thermalPalette.Lock();
  TLockedToken< CTexture > token = xc_objStore.GetObj("TXTR_ThermoPalette");
  const CGraphicsPalette* pal = token->GetPalette();
  for (int i = 0; i < 16; ++i) {
    ushort& dst = x288_thermalPalette.GetPaletteData()[i];
    dst = pal ? pal->GetPaletteData()[i] : 0;
  }
  x288_thermalPalette.UnLock();
}

CCubeRenderer::~CCubeRenderer() {
  sRenderer = nullptr;
  Buckets::Shutdown();
  CSkinnedModel::RemoveDummySkinnedModelRef();
  if (!x314_phazonSuitMask.null()) {
    x314_phazonSuitMask->ScheduleDeletion();
  }
}

void CCubeRenderer::AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                                      const CAreaRenderOctTree* octTree, int areaIdx) {
  if (FindStaticGeometry(geometry) == x1c_areaListItems.end()) {
    rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > > models =
        rs_new rstl::vector< rstl::auto_ptr< CCubeModel > >();
    rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > > textures =
        rs_new rstl::vector< TCachedToken< CTexture > >();
    if (!geometry->empty()) {
      CCubeModel::MakeTexturesFromMats(geometry->front().GetMaterialPointer(), *textures,
                                       xc_objStore, false);
      models->reserve(geometry->size());
      for (int i = 0; i < geometry->size(); ++i) {
        const CMetroidModelInstance* it = &geometry->at(i);
        models->push_back(rs_new CCubeModel(
            const_cast< rstl::vector< void* >* >(&it->GetSurfaces()), textures.get(),
            it->GetMaterialPointer(), it->GetVertexPointer(), it->GetNormalPointer(),
            it->GetColorPointer(), it->GetTCPointer(), it->GetPackedTCPointer(),
            it->GetBoundingBox(), it->GetFlags(), false, i));
      }
    }
    x1c_areaListItems.push_back(CAreaListItem(geometry, octTree, textures, models, areaIdx));
    GXInvalidateVtxCache();
  }
}

rstl::list< CCubeRenderer::CAreaListItem >::iterator
CCubeRenderer::FindStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) {
  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    if (it->x0_geometry == geometry) {
      return it;
    }
  }
  return x1c_areaListItems.end();
}

void CCubeRenderer::RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) {
  AUTO(search, FindStaticGeometry(geometry));
  if (search != x1c_areaListItems.end()) {
    x1c_areaListItems.erase(search);
  }
}

void CCubeRenderer::SetModelMatrix(const CTransform4f& xf) { CGraphics::SetModelMatrix(xf); }

void CCubeRenderer::SetWorldViewpoint(const CTransform4f& xf) {
  CGraphics::SetViewPointMatrix(xf);
  CVector3f normal = xf.GetForward();
  xb0_viewPlane.SetFrom((normal.GetX() * xf.Get03()) + (normal.GetY() * xf.Get13()) +
                            (normal.GetZ() * xf.Get23()),
                        normal);
}

void CCubeRenderer::BeginScene() {
  int width = CGraphics::GetViewport().mWidth;
  int height = CGraphics::GetViewport().mHeight;
  CGraphics::SetUseVideoFilter(true);
  CGraphics::SetViewport(0, 0, width, height);
  CGraphics::SetClearColor(CColor(static_cast< uchar >(0), 0, 0, 0));
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
#if NONMATCHING
  // TODO: double check this
  float aspect = static_cast< float >(width) / static_cast< float >(height);
#else
  float aspect = 1.3333334f;
#endif
  CGraphics::SetPerspective(75.f, aspect, 1.f, 4096.f);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::TickRenderTimings();
  if (x310_phazonSuitMaskCountdown != 0) {
    --x310_phazonSuitMaskCountdown;
    if (x310_phazonSuitMaskCountdown == 0) {
      x314_phazonSuitMask->ScheduleDeletion();
      x314_phazonSuitMask = nullptr;
    }
  }

  x318_27_currentRGBA6 = x318_26_requestRGBA6;
  if (!x318_31_persistRGBA6) {
    x318_26_requestRGBA6 = false;
  }

  GXSetPixelFmt(x318_27_currentRGBA6 ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GXSetAlphaUpdate(GX_TRUE);
  GXSetDstAlpha(GX_TRUE, 0);
  CGraphics::BeginScene();
}

void CCubeRenderer::EndScene() {
  x318_31_persistRGBA6 = !CGraphics::IsBeginSceneClearFb();
  CGraphics::EndScene();
  if (x2dc_reflectionAge >= 2) {
    x14c_reflectionTex = nullptr;
  } else {
    ++x2dc_reflectionAge;
  }
}

void CCubeRenderer::AddParticleGen(const CParticleGen& gen) {
  AUTO(bounds, gen.GetBounds());
  if (bounds) {
    CVector3f closestPoint = bounds->ClosestPointAlongVector(GetViewPlane().GetNormal());
    Buckets::Insert(closestPoint, *bounds, kDT_Particle, static_cast< const void* >(&gen),
                    GetViewPlane(), 0);
  }
}

void CCubeRenderer::AddParticleGen(const CParticleGen& gen, const CVector3f& pos,
                                   const CAABox& bounds) {
  Buckets::Insert(pos, bounds, kDT_Particle, static_cast< const void* >(&gen), GetViewPlane(), 0);
}

void CCubeRenderer::AddPlaneObject(const void* obj, const CAABox& aabb, const CPlane& plane,
                                   int type) {
  static const CVector3f sOptimalPlane(0.f, 0.f, 1.f);

  CVector3f closestPoint = aabb.ClosestPointAlongVector(GetViewPlane().GetNormal());
  float closestDist = GetViewPlane().GetHeight(closestPoint);
  CVector3f furthestPoint = aabb.FurthestPointAlongVector(GetViewPlane().GetNormal());
  float furthestDist = GetViewPlane().GetHeight(furthestPoint);
  if (closestDist < 0.f && furthestDist < 0.f) {
    return;
  }

  bool zOnly;
  if (plane.GetNormal() == sOptimalPlane) {
    zOnly = true;
  } else {
    zOnly = false;
  }

  bool invertTest;
  if (zOnly) {
    if (CGraphics::GetViewMatrix().GetTranslation().GetZ() >= plane.GetConstant()) {
      invertTest = true;
    } else {
      invertTest = false;
    }
  } else if (plane.GetHeight(CGraphics::GetViewMatrix().GetTranslation()) >= 0.f) {
    invertTest = true;
  } else {
    invertTest = false;
  }

  Buckets::InsertPlaneObject(closestDist, furthestDist, aabb, invertTest, plane, zOnly,
                             EDrawableType(type + 2), obj);
}

void CCubeRenderer::AddDrawable(const void* obj, const CVector3f& pos, const CAABox& aabb, int mode,
                                IRenderer::EDrawableSorting sorting) {
  if (sorting == IRenderer::kDS_UnsortedCallback) {
    xa8_drawableCallback(obj, xac_drawableCallbackUserData, mode);
  } else {
    Buckets::Insert(pos, aabb, EDrawableType(mode + 2), obj, GetViewPlane(), 0);
  }
}

void CCubeRenderer::SetupRendererStates(bool depthWrite) {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetAmbientColor(CColor(0));
  CGraphics::SetDepthWriteMode(true, kE_LEqual, depthWrite);
  CCubeMaterial::ResetCachedMaterials();
  GXSetTevColor(GX_TEVREG1, x2fc_tevReg1Color.GetGXColor());
}

void CCubeRenderer::SetupCGraphicsStates() {
  const GXColor sWhite = {255, 255, 255, 255};
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CTevCombiners::ResetStates();
  CGraphics::SetAmbientColor(CColor(0.4f, 0.4f, 0.4f, 1.f));
  CGX::SetChanMatColor(CGX::Channel0, sWhite);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CCubeMaterial::EnsureTevsDirect();
}

void CCubeRenderer::AddWorldSurfaces(CCubeModel& model) {
  const CPlane& viewPlane = xb0_viewPlane;
  for (CCubeSurface it = model.GetAlphaSurfaces(); it.IsValid(); it = it.GetNextSurface()) {
    uint blend = model.GetMaterialByIndex(it.GetMaterialIndex()).GetCompressedBlend();
    const CAABox bounds = it.GetBounds();
    void* surfData = it.x0_data;
    Buckets::Insert(bounds.ClosestPointAlongVector(viewPlane.GetNormal()), bounds, kDT_WorldSurface,
                    surfData, viewPlane, blend == 0x50004 ? 1 : 0);
  }
}

void CCubeRenderer::DrawRenderBucketsDebug() {}

void CCubeRenderer::RenderBucketItems(const CAreaListItem* areaListItem) {
  AUTO_CONST_REF(bucketData, Buckets::GetBucketData());
  AUTO_CONST_REF(bucketHolder, Buckets::GetBucketHolder());

  int lastType = -1;
  CCubeModel* currentModel = nullptr;

  for (AUTO(bucketIdx, bucketData.begin()); bucketIdx != bucketData.end(); ++bucketIdx) {
    const Buckets::BucketHolderType& bucket = bucketHolder[*bucketIdx];
    for (AUTO(drawableIt, bucket.begin()); drawableIt != bucket.end(); ++drawableIt) {
      CDrawable* drawable = *drawableIt;
      const int drawType = drawable->GetType();

      switch (drawType) {
      case kDT_Particle: {
        if (lastType != drawType) {
          SetupCGraphicsStates();
        }
        static_cast< CParticleGen* >(drawable->GetData())->Render();
        break;
      }
      case kDT_WorldSurface: {
        if (lastType != drawType) {
          SetupRendererStates(false);
          currentModel = nullptr;
        }
        CCubeSurface surface(drawable->GetData());
        CCubeModel* model = surface.x0_data->mParent;
        if (model != currentModel) {
          model->SetArraysCurrent();
          currentModel = model;
          ActivateLightsForModel(areaListItem, *model);
        }
        model->DrawSurface(surface, skNormalFlagNoUpdate);
        break;
      }
      default: {
        if (lastType != drawType) {
          CCubeMaterial::EnsureTevsDirect();
        }
        if (xa8_drawableCallback != nullptr) {
          xa8_drawableCallback(drawable->GetData(), xac_drawableCallbackUserData,
                               drawable->GetType() - kDT_Actor);
        }
        break;
      }
      }
      lastType = drawType;
    }
  }
}

void CCubeRenderer::HandleUnsortedModel(const CAreaListItem* areaListItem, CCubeModel& model) {
  void* surfPtr = model.GetNormalSurfaces().x0_rawdata;
  if (surfPtr == nullptr) {
    return;
  }

  model.SetArraysCurrent();
  ActivateLightsForModel(areaListItem, model);

  CCubeSurface surf(surfPtr);
  do {
    model.DrawSurface(surf, skNormalFlag);
    surf = surf.GetNextSurface();
  } while (surf.IsValid());
}

void CCubeRenderer::HandleUnsortedModelWireframe(const CAreaListItem* areaListItem,
                                                 CCubeModel& model) {
  void* unsortedPtr = model.GetNormalSurfaces().x0_rawdata;
  void* sortedPtr = model.GetAlphaSurfaces().x0_rawdata;

  model.SetArraysCurrent();
  ActivateLightsForModel(areaListItem, model);

  CCubeSurface surf(unsortedPtr);
  for (; surf.IsValid(); surf = surf.GetNextSurface()) {
    model.DrawSurfaceWireframe(surf);
  }

  surf = CCubeSurface(sortedPtr);
  for (; surf.IsValid(); surf = surf.GetNextSurface()) {
    model.DrawSurfaceWireframe(surf);
  }
}

void CCubeRenderer::DrawUnsortedGeometry(int areaIdx, uint mask, uint targetMask) {
  const CAreaListItem* areaListItem = nullptr;

  SetupRendererStates(true);

  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    CAreaListItem& item = *it;
    if (areaIdx != -1 && areaIdx != item.GetAreaId()) {
      continue;
    }

    if (item.x4_octTree != nullptr) {
      areaListItem = &item;
    }

    AUTO_REF(models, *item.GetModelList());

    const CPVSVisSet* pvs = xc8_pvsVisSet ? xc8_pvsVisSet.get_ptr() : nullptr;
    if (xe0_pvsAreaIdx != item.GetAreaId()) {
      pvs = nullptr;
    }

    int modelIdx = 0;
    for (AUTO(modelIt, models.begin()); modelIt != models.end(); ++modelIt, ++modelIdx) {
      CCubeModel& model = **modelIt;

      if (pvs != nullptr) {
        const bool visible = pvs->GetVisible(modelIdx) != kVSS_EndOfTree;
        if ((xc4_pvsState == 1 && !visible) || (xc4_pvsState == 2 && visible)) {
          model.SetShouldDrawWorldFlag(false);
          continue;
        }
      }

      if (targetMask != (mask & model.GetModelFlags())) {
        model.SetShouldDrawWorldFlag(false);
        continue;
      }

      if (!x44_frustumPlanes.BoxInFrustumPlanes(model.GetBoundingBox())) {
        model.SetShouldDrawWorldFlag(false);
        continue;
      }

      if (x318_25_drawWireframe) {
        model.SetShouldDrawWorldFlag(false);
        HandleUnsortedModelWireframe(areaListItem, model);
      } else {
        model.SetShouldDrawWorldFlag(true);
        HandleUnsortedModel(areaListItem, model);
      }
    }
  }

  SetupCGraphicsStates();
}

void CCubeRenderer::DrawSortedGeometry(int areaIdx, uint mask, uint targetMask) {
  (void)mask;
  (void)targetMask;

  SetupRendererStates(true);

  const CAreaListItem* areaListItem = nullptr;
  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    CAreaListItem& item = *it;
    if (areaIdx != -1 && areaIdx != item.GetAreaId()) {
      continue;
    }

    if (item.x4_octTree != nullptr) {
      areaListItem = &item;
    }

    rstl::vector< rstl::auto_ptr< CCubeModel > >& models = *item.GetModelList();
    for (AUTO(modelIt, models.begin()); modelIt != models.end(); ++modelIt) {
      CCubeModel& model = **modelIt;
      if (model.GetShouldDrawWorldFlag()) {
        AddWorldSurfaces(model);
      }
    }
  }

  Buckets::Sort();
  RenderBucketItems(areaListItem);
  SetupCGraphicsStates();
  DrawRenderBucketsDebug();
  Buckets::Clear();
}

void CCubeRenderer::DrawStaticGeometry(int areaIdx, uint mask, uint targetMask) {
  CCubeRenderer::DrawUnsortedGeometry(areaIdx, mask, targetMask);
  CCubeRenderer::DrawSortedGeometry(areaIdx, mask, targetMask);
}

void CCubeRenderer::ActivateLightsForModel(const CAreaListItem* areaListItem,
                                           const CCubeModel& model) {
  uchar lightState = 0;

  if (!x300_dynamicLights.empty()) {
    int addedLights[4];
    float distances[4] = {-1.f, -1.f, -1.f, -1.f};

    const CAABox& modelAABB = model.GetBoundingBox();

    uint octreeWordCount = 0;
    const uint* octreeWords = nullptr;
    int loadedLightCount = 0;
    if (areaListItem != nullptr && model.GetModelIndex() != -1) {
      octreeWords = areaListItem->x1c_lightOctreeWords.data();
      octreeWordCount = areaListItem->x4_octTree->x14_bitmapWordCount;
    }

    const uint* curOctreeWords = octreeWords;
    for (int i = 0; i < x300_dynamicLights.size() && loadedLightCount < 4;
         ++i, curOctreeWords += octreeWordCount) {
      const CLight& light = x300_dynamicLights[i];
      if (octreeWords == nullptr ||
          CAreaRenderOctTree::TestBit(curOctreeWords, model.GetModelIndex())) {
        bool replacedLight = false;

        for (int j = 0; j < loadedLightCount; ++j) {
          if (addedLights[j] == light.GetId()) {
            const float radius = CollisionUtil::AABoxSphereIntersectionRadius(
                modelAABB, CSphere(light.GetPosition(), light.GetRadius()));
            if (radius >= 0.f && distances[j] > radius) {
              distances[j] = radius;
              CGraphics::LoadLight(ERglLight(j), light);
              replacedLight = true;
            }
            break;
          }
        }

        if (!replacedLight) {
          const float radius = CollisionUtil::AABoxSphereIntersectionRadius(
              modelAABB, CSphere(light.GetPosition(), light.GetRadius()));
          distances[loadedLightCount] = radius;
          if (distances[loadedLightCount] >= 0.f) {
            CGraphics::LoadLight(ERglLight(loadedLightCount), light);
            lightState = lightState | (1 << loadedLightCount);
            addedLights[loadedLightCount] = light.GetId();
            ++loadedLightCount;
          }
        }
      }
    }
  }

  if (lightState != 0) {
    const GXColor color = {0xFF, 0xFF, 0xFF, 0xFF};
    CGraphics::SetLightState(static_cast< uchar >(lightState));
    CGX::SetChanMatColor(CGX::Channel0, color);
  } else {
    CGraphics::DisableAllLights();
    GXColor color = CGX::GetChanAmbColor(CGX::Channel0);
    CGX::SetChanMatColor(CGX::Channel0, color);
  }
}

namespace Renderer {
IRenderer* AllocateRenderer(IObjectStore& objStore, COsContext& osContext, CMemorySys& memorySys,
                            CResFactory& resFactory) {
  CCubeRenderer* renderer = rs_new CCubeRenderer(objStore, osContext, memorySys, resFactory);
  IWeaponRenderer::SetRenderer(renderer);
  return renderer;
}
} // namespace Renderer

void CCubeRenderer::PrimColor(float r, float g, float b, float a) {
  x2e0_primColor.Set(r, g, b, a);
}

void CCubeRenderer::PrimColor(const CColor& color) { x2e0_primColor = color; }

void CCubeRenderer::BeginPrimitive(IRenderer::EPrimitiveType type, int nverts) {
  const GXVtxDescList vtxDescList[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_NRM, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  x18_primVertCount = nverts;
  CGX::SetVtxDescv(vtxDescList);
  CGX::Begin(GXPrimitive(type), GX_VTXFMT0, nverts);
}

void CCubeRenderer::BeginLines(int nverts) {
  CCubeRenderer::BeginPrimitive(IRenderer::kPT_Lines, nverts);
}

void CCubeRenderer::BeginLineStrip(int nverts) {
  CCubeRenderer::BeginPrimitive(IRenderer::kPT_LineStrip, nverts);
}

void CCubeRenderer::BeginTriangles(int nverts) {
  CCubeRenderer::BeginPrimitive(IRenderer::kPT_Triangles, nverts);
}

void CCubeRenderer::BeginTriangleStrip(int nverts) {
  CCubeRenderer::BeginPrimitive(IRenderer::kPT_TriangleStrip, nverts);
}

void CCubeRenderer::BeginTriangleFan(int nverts) {
  CCubeRenderer::BeginPrimitive(IRenderer::kPT_TriangleFan, nverts);
}

void CCubeRenderer::PrimVertex(const CVector3f& vtx) {
  --x18_primVertCount;
  GXPosition3f32(vtx.GetX(), vtx.GetY(), vtx.GetZ());
  GXNormal3f32(x2e4_primNormal.GetX(), x2e4_primNormal.GetY(), x2e4_primNormal.GetZ());
  GXColor1u32(x2e0_primColor.GetColor_u32());
}

void CCubeRenderer::PrimNormal(const CVector3f& nrm) { x2e4_primNormal = nrm; }

void CCubeRenderer::EndPrimitive() {
  while (x18_primVertCount != 0) {
    PrimVertex(CVector3f::Zero());
  }
  CGX::End();
}

void CCubeRenderer::SetAmbientColor(const CColor& color) { CGraphics::SetAmbientColor(color); }

void CCubeRenderer::SetPerspective(float fovy, float width, float height, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, width / height, znear, zfar);
}

void CCubeRenderer::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, aspect, znear, zfar);
}

rstl::pair< CVector2f, CVector2f > CCubeRenderer::SetViewportOrtho(bool centered, float znear,
                                                                   float zfar) {
  const CViewport& vp = CGraphics::GetViewport();
  int vpLeft = vp.mLeft;
  int vpTop = vp.mTop;
  int vpWidth = vp.mWidth;
  int vpHeight = vp.mHeight;
  float left = static_cast< float >(centered ? vpLeft - vpWidth / 2 : vpLeft);
  float top = static_cast< float >(centered ? vpTop - vpHeight / 2 : vpTop);
  float right = static_cast< float >(centered ? vpLeft + vpWidth / 2 : vpLeft + vpWidth);
  float bottom = static_cast< float >(centered ? vpTop + vpHeight / 2 : vpTop + vpHeight);
  CGraphics::SetOrtho(left, right, bottom, top, znear, zfar);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  return rstl::pair< CVector2f, CVector2f >(CVector2f(left, top), CVector2f(right, bottom));
}

void CCubeRenderer::SetViewport(int left, int bottom, int width, int height) {
  CGraphics::SetViewport(left, bottom, width, height);
  CGraphics::SetScissor(left, bottom, width, height);
}

void CCubeRenderer::SetDepthReadWrite(const bool read, const bool update) {
  CGraphics::SetDepthWriteMode(read, kE_LEqual, update);
}

void CCubeRenderer::SetBlendMode_AdditiveAlpha() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_AlphaBlended() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_NoColorWrite() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_Zero, kBF_One, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_ColorMultiply() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_Zero, kBF_SrcColor, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_InvertDst() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_InvSrcColor, kBF_Zero, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_InvertSrc() {
  CGraphics::SetBlendMode(kBM_Logic, kBF_One, kBF_Zero, kLO_InvCopy);
}

void CCubeRenderer::SetBlendMode_Replace() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_AdditiveDestColor() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcColor, kBF_One, kLO_Clear);
}

void CCubeRenderer::SetClippingPlanes(const CFrustumPlanes& frustum) {
  x44_frustumPlanes = frustum;
}

float CCubeRenderer::GetFPS() { return CGraphics::GetFPS(); }

void CCubeRenderer::SetDrawableCallback(IRenderer::TDrawableCallback cb, const void* ctx) {
  xa8_drawableCallback = cb;
  xac_drawableCallbackUserData = ctx;
}

void CCubeRenderer::SetDebugOption(IRenderer::EDebugOption option, int value) {
  switch (option) {
  case IRenderer::kDO_PVSMode:
    xc0_pvsMode = value != 0 ? 1 : 0;
    break;
  case IRenderer::kDO_PVSState:
    xc4_pvsState = value;
    break;
  case IRenderer::kDO_FogDisabled:
    x318_28_disableFog = value != 0;
    break;
  default:
    break;
  }
}

CTexture* CCubeRenderer::GetRealReflection() {
  x2dc_reflectionAge = 0;
  if (!x14c_reflectionTex.null()) {
    return x14c_reflectionTex.get();
  }
  return &xe4_blackTex;
}

void CCubeRenderer::CacheReflection(void (*cb)(void*, const CVector3f&), void* ctx,
                                    bool clearAfter) {
  if (x318_24_reflectionDirty) {
    x318_24_reflectionDirty = false;
    x2dc_reflectionAge = 0;

    if (x14c_reflectionTex.null()) {
      x14c_reflectionTex = rs_new CTexture(kTF_RGB565, 0x80, 0x80, 1);
    }

    const CViewport& vp = CGraphics::GetViewport();
    const int oldLeft = vp.mLeft;
    const int oldTop = vp.mTop;
    const int oldWidth = vp.mWidth;
    const int oldHeight = vp.mHeight;

    const int captureTop = static_cast< int >(CGraphics::mRenderModeObj.efbHeight) - 0x100;
    CGraphics::SetViewport(0, captureTop, 0x100, 0x100);
    CGraphics::SetScissor(0, captureTop, 0x100, 0x100);

    void* spareBuffer = CGraphics::mpSpareBuffer;
    GXSetTexCopySrc(0, 0, 0x100, 0x100);
    GXSetTexCopyDst(0x80, 0x80, GX_TF_RGB565, true);
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(spareBuffer, true);

    cb(ctx, CCubeMaterial::GetViewingReflection());

    void* reflectionData = const_cast< void* >(x14c_reflectionTex->GetConstBitMapData(0));
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(reflectionData, clearAfter);

    CGraphics::SetViewport(oldLeft, oldTop, oldWidth, oldHeight);
    CGraphics::SetScissor(oldLeft, oldTop, oldWidth, oldHeight);
  }
}

void CCubeRenderer::DrawSpaceWarp(const CVector3f& point, float strength) {
  if (!(point.GetZ() >= 1.f)) {
    _DrawSpaceWarp(point, strength);
  }
}

void CCubeRenderer::_DrawSpaceWarp(const CVector3f& point, float strength) {
  const int vpLeft = CGraphics::GetViewport().mLeft;
  const int vpTop = CGraphics::GetViewport().mTop;
  const int vpWidth = CGraphics::GetViewport().mWidth;
  const int vpHeight = CGraphics::GetViewport().mHeight;

  CVector3f projectedPoint = point;
  float& screenY = projectedPoint[1];
  float& screenX = projectedPoint[0];
  screenX = static_cast< float >(vpLeft) +
            (static_cast< float >(vpWidth / 2) * screenX + static_cast< float >(vpWidth / 2));
  screenY = static_cast< float >(vpTop) +
            (static_cast< float >(vpHeight / 2) * -screenY + static_cast< float >(vpHeight / 2));

  CVector2i center(static_cast< int >(screenX) & ~3, static_cast< int >(screenY) & ~3);
  CVector2i minPoint = center - CVector2i(0x60, 0x60);
  CVector2i maxPoint = center + CVector2i(0x60, 0x60);

  int& minX = minPoint[0];
  int& maxX = maxPoint[0];
  CVector2f uv1min(0.f, 0.f);
  float& uMin = uv1min[0];
  CVector2f uv1max(1.f, 1.f);
  float& uMax = uv1max[0];

  const int alignedLeft = vpLeft & ~3;
  const int alignedTop = vpTop & ~3;
  const int alignedRight = (3 + (vpLeft + vpWidth)) & ~3;
  const int alignedBottom = (3 + (vpTop + vpHeight)) & ~3;

  if (minX < alignedLeft) {
    uv1min[0] = 0.0052083335f * static_cast< float >(alignedLeft - minPoint.GetX());
    minPoint.SetX(alignedLeft);
  }

  if (minPoint[1] < alignedTop) {
    uv1min[1] = 0.0052083335f * static_cast< float >(alignedTop - minPoint.GetY());
    minPoint.SetY(alignedTop);
  }

  if (maxX > alignedRight) {
    uv1max[0] = 1.f - 0.0052083335f * static_cast< float >(maxPoint.GetX() - alignedRight);
    maxPoint.SetX(alignedRight);
  }

  if (maxPoint[1] > alignedBottom) {
    uv1max[1] = 1.f - 0.0052083335f * static_cast< float >(maxPoint.GetY() - alignedBottom);
    maxPoint.SetY(alignedBottom);
  }

  CVector2i dimensions = maxPoint - minPoint;
  int& sizeX = dimensions[0];
  int& sizeY = dimensions[1];
  if (sizeX <= 0 || sizeY <= 0) {
    return;
  }

  GXFogType fogType;
  float fogStartZ;
  float fogEndZ;
  float fogNearZ;
  float fogFarZ;
  GXColor fogColor;

  CGX::GetFog(&fogType, &fogStartZ, &fogEndZ, &fogNearZ, &fogFarZ, &fogColor);
  CGX::SetFog(GX_FOG_NONE, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);

  void* spareBuffer = CGraphics::mpSpareBuffer;
  GXSetTexCopySrc(minPoint.GetX(), minPoint.GetY(), sizeX, sizeY);
  GXSetTexCopyDst(sizeX, sizeY, GX_TF_RGBA8, false);
  GXCopyTex(spareBuffer, false);
  GXPixModeSync();

  CGraphics::LoadDolphinSpareTexture(sizeX, sizeY, GX_TF_RGBA8, 0, CGraphics::kSpareBufferTexMapID);

  x150_reflectionTex.Load(GX_TEXMAP1, CTexture::kCM_Clamp);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

  const float indScale = static_cast< float >(0.5 * strength);
  float indMtx[2][3] = {
      {indScale, 0.f, 0.f},
      {0.f, indScale, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, -1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(2);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

  static const GXVtxDescList vtxDescrs[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);

  CGraphics::CProjectionState backupProj(CGraphics::GetProjectionState());
  CTransform4f backupViewMtx(CGraphics::mViewMatrix);

  CGraphics::SetOrtho(static_cast< float >(vpLeft), static_cast< float >(vpWidth + vpLeft),
                      static_cast< float >(vpTop), static_cast< float >(vpHeight + vpTop), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(static_cast< float >(minPoint.GetX()), 0.5f,
                 static_cast< float >(minPoint.GetY()));
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(uMin, uv1min[1]);

  GXPosition3f32(static_cast< float >(minPoint.GetX()), 0.5f,
                 static_cast< float >(maxPoint.GetY()));
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(uMin, uv1max[1]);

  GXPosition3f32(static_cast< float >(maxPoint.GetX()), 0.5f,
                 static_cast< float >(maxPoint.GetY()));
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(uMax, uv1max[1]);

  GXPosition3f32(static_cast< float >(maxPoint.GetX()), 0.5f,
                 static_cast< float >(minPoint.GetY()));
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(uMax, uv1min[1]);

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProj);
  CGraphics::SetViewPointMatrix(backupViewMtx);

  CGX::SetFog(fogType, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);
}

void CCubeRenderer::SetWireframeFlags(int flags) {
  CCubeModel::SetModelWireframe((flags & 1) == 1);
  x318_25_drawWireframe = (flags & 2) == 2;
}

void CCubeRenderer::SetWorldFog(ERglFogMode mode, float startz, float endz, const CColor& color) {
  if (x318_28_disableFog) {
    mode = static_cast< ERglFogMode >(0);
  }
  CGraphics::SetFog(mode, startz, endz, color);
}

int CCubeRenderer::GetStaticWorldDataSize() {
  int size = 0;
  rstl::list< CAreaListItem >::const_iterator it = x1c_areaListItems.begin();
  rstl::list< CAreaListItem >::const_iterator end = x1c_areaListItems.end();
  for (; it != end; ++it) {
    const rstl::vector< TCachedToken< CTexture > >* const textures = it->x8_textures.get();
    if (textures) {
      size += textures->size() * 0xc;
    }
  }
  return size;
}

void CCubeRenderer::DrawFogFan(const CVector3f* verts, int vertCount) {
  if (vertCount < 3) {
    return;
  }

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast< ushort >(vertCount));
  for (int i = 0; i < vertCount; ++i) {
    GXPosition3f32(verts[i].GetX(), verts[i].GetY(), verts[i].GetZ());
  }
  CGX::End();
}

void CCubeRenderer::DrawFogFans(const CPlane* planes, int planeCount, const CVector3f* verts,
                                int vertCount, int startPlane, int curPlane) {
  if (curPlane == startPlane) {
    DrawFogFans(planes, planeCount, verts, vertCount, startPlane, curPlane + 1);
  } else if (curPlane == planeCount) {
    DrawFogFan(verts, vertCount);
  } else {
    rstl::reserved_vector< CVector3f, 20 > clippedVerts;
    rstl::reserved_vector< bool, 20 > clippedFlags;
    const CPlane& plane = planes[curPlane];
    for (int i = 0; i < vertCount; ++i) {
      clippedFlags.push_back(!plane.IsFacing(verts[i]));
    }

    for (int i = 0; i < vertCount; ++i) {
      const int next = i != vertCount - 1 ? i + 1 : 0;
      const int clippedMask = clippedFlags[i] | (clippedFlags[next] << 1);
      if ((clippedMask & 1) == 0) {
        clippedVerts.push_back(verts[i]);
      }
      if (clippedMask == 1 || clippedMask == 2) {
        const float t = plane.ClipLineSegment(verts[i], verts[next]);
        if (t > 0.f && t < 1.f) {
          clippedVerts.push_back(CVector3f::Lerp(verts[i], verts[next], t));
        }
      }
    }

    if (clippedVerts.size() >= 3) {
      DrawFogFans(planes, planeCount, clippedVerts.data(), clippedVerts.size(), startPlane,
                  curPlane + 1);
    }
  }
}

void CCubeRenderer::DrawFogSlices(const CPlane* planes, int planeCount, int planeIdx,
                                  const CVector3f& point, float extent) {
  static const int skEdges[3][2] = {{1, 2}, {0, 2}, {0, 1}};
  const CPlane& plane = planes[planeIdx];
  rstl::reserved_vector< CVector3d, 4 > doubleCorners;
  rstl::reserved_vector< CVector3f, 4 > corners;

  int axis = 0;
  if (fabs(plane.GetNormal().GetY()) > fabs(plane.GetNormal().GetX())) {
    axis = 1;
  }
  if (fabs(plane.GetNormal().GetZ()) > fabs(plane.GetNormal()[axis])) {
    axis = 2;
  }

  const CVector3d projectedPoint(point - plane.GetHeight(point) * plane.GetNormal());
  float negative = -1.f;
  float positive = 1.f;
  float axisSign = CMath::FastFSel(plane.GetNormal()[axis], negative, positive);
  if (axis == 1) {
    axisSign = -axisSign;
  }
  CVector3d offsetA(0.0, 0.0, 0.0);
  CVector3d offsetB(0.0, 0.0, 0.0);
  offsetA[skEdges[axis][0]] = extent;
  offsetB[skEdges[axis][1]] = extent * axisSign;
  doubleCorners.push_back(projectedPoint - offsetA - offsetB);
  doubleCorners.push_back(projectedPoint + offsetA - offsetB);
  doubleCorners.push_back(projectedPoint + offsetA + offsetB);
  doubleCorners.push_back(projectedPoint - offsetA + offsetB);

  for (int i = 0; i < 4; ++i) {
    const CVector3d normal(plane.GetNormal());
    const CVector3d& corner = doubleCorners[i];
    const double height = corner.GetX() * normal.GetX() + corner.GetY() * normal.GetY() +
                          corner.GetZ() * normal.GetZ() - plane.GetConstant();
    const CVector3d projected = corner - height * normal;
    corners.push_back(CVector3f(projected.GetX(), projected.GetY(), projected.GetZ()));
  }
  DrawFogFans(planes, planeCount, corners.data(), doubleCorners.size(), planeIdx, 0);
}

static void draw_box_or_model(const CAABox& aabb, const CModel* model, const CTransform4f& modelXf,
                              const CTransform4f& viewXf, const CSkinnedModel* skinnedModel) {
  if (model == nullptr && skinnedModel == nullptr) {
    const CAABox transformedAabb = aabb.GetTransformedAABox(modelXf);
    CAABox worldAabb = transformedAabb;

    static const GXVtxDescList vtxDescrs2[2] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    CGX::SetVtxDescv(vtxDescrs2);

    const CUnitVector3f viewPlaneFwd(viewXf.GetForward());

    CVector3f max = worldAabb.GetMaxPoint();
    max.SetX(-max.GetX());
    max.SetY(-max.GetY());
    max.SetZ(-max.GetZ());

    CPlane fogPlanes[7] = {
        CPlane(worldAabb.GetMinPoint().GetX(), CVector3f::Right()),
        CPlane(max.GetX(), CVector3f::Left()),
        CPlane(worldAabb.GetMinPoint().GetY(), CVector3f::Forward()),
        CPlane(max.GetY(), CVector3f::Back()),
        CPlane(worldAabb.GetMinPoint().GetZ(), CVector3f::Up()),
        CPlane(max.GetZ(), CVector3f::Down()),
        CPlane(CVector3f::Dot(viewXf.GetTranslation(), viewPlaneFwd) + 0.2f + 0.1f, viewPlaneFwd),
    };
    CGraphics::SetModelMatrix(CTransform4f::Identity());

    float maxExtent = rstl::max_val(rstl::max_val(worldAabb.GetDepth(), worldAabb.GetHeight()),
                                    worldAabb.GetWidth());

    const float sliceExtent = maxExtent * 2.f;
    for (int i = 0; i < 7; ++i) {
      CCubeRenderer::DrawFogSlices(fogPlanes, 7, i, worldAabb.GetCenterPoint(), sliceExtent);
    }
  } else if (skinnedModel != nullptr) {
    const CModel* skModel = *skinnedModel->GetModel();
    skModel->Touch(0);
    if (const CCubeModel* modelInst = skModel->GetCubeModel()) {
      skModel->UpdateLastFrame();
      const CModel& normalModel = **skinnedModel->GetModel();
      const float* const& normals = normalModel.GetNormals();
      const CModel& positionModel = **skinnedModel->GetModel();
      modelInst->DrawFlat(positionModel.GetPositions(), normals, kSS_All);
    }
  } else {
    model->Touch(0);
    if (const CCubeModel* modelInst = model->GetCubeModel()) {
      model->UpdateLastFrame();
      modelInst->DrawFlat(nullptr, nullptr, kSS_All);
    }
  }
}

void CCubeRenderer::DoThermalBlendCold() {
  const float coldScale = 0.003921569f * static_cast< float >(x2f8_thermalColdScale);
  x318_26_requestRGBA6 = true;

  GXSetAlphaUpdate(true);
  GXSetDstAlpha(false, 0);

  const int left = CGraphics::mViewport.mLeft;
  const int top = CGraphics::mViewport.mTop;
  const int width = CGraphics::mViewport.mWidth;
  const int height = CGraphics::mViewport.mHeight;
  void* dest = CGraphics::mpSpareBuffer;

  CGX::SetZMode(true, GX_LEQUAL, false);
  GXSetTexCopySrc(static_cast< u16 >(left), static_cast< u16 >(top), static_cast< u16 >(width),
                  static_cast< u16 >(height));
  GXSetTexCopyDst(static_cast< u16 >(width), static_cast< u16 >(height), GX_TF_I4, false);
  GXCopyTex(dest, true);

  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_I4, 0, CGraphics::kSpareBufferTexMapID);

  const uint rand = x2a8_thermalRand.Next();
  void* randTexData = reinterpret_cast< void* >(((rand + 0x1f) & ~0x1f) + 0x8000);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_IA4, randTexData, GX_TEXMAP0);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_IA4, randTexData, GX_TEXMAP1);

  const float& thermalVisorLevel = 0.5f * x2f0_thermalVisorLevel;
  CMath::Clamp(0.f, thermalVisorLevel, 0.5f);

  float indMtx[2][3] = {
      {0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f},
  };
  indMtx[0][0] = static_cast< float >(0.1 * (1.f - coldScale));
  indMtx[1][1] = indMtx[0][0];
  GXSetIndTexMtx(GX_ITM_0, indMtx, -2);

  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);

  const uint scale = x2f8_thermalColdScale;
  int tr1Alpha = 0xff;
  if (scale < 0x80) {
    tr1Alpha = (scale & 0x7f) << 1;
  }

  uchar tr2 = 0;
  if (scale < 0x40) {
    tr2 = 0;
  } else if (scale == 0xff) {
    tr2 = 0xff;
  } else {
    tr2 = static_cast< uchar >((static_cast< int >(scale - 0x40) * 4) / 3);
  }

  static const GXColor tr1ClrZero = {0, 0, 0, 0};
  static const GXColor tr2ClrZero = {0, 0, 0, 0};
  GXColor tr1Clr = tr1ClrZero;
  GXColor tr2Clr = tr2ClrZero;
  const uchar tr1Rgb = static_cast< uchar >((0xff - static_cast< uchar >(tr1Alpha)) >> 3);
  tr1Clr.r = tr1Rgb;
  tr1Clr.g = tr1Rgb;
  tr1Clr.b = tr1Rgb;
  tr1Clr.a = static_cast< uchar >(tr1Alpha);

  tr2Clr.r = tr2;
  tr2Clr.g = tr2;
  tr2Clr.b = tr2;
  tr2Clr.a = tr2;

  const CColor lerpedColor =
      CColor::Lerp(x2f4_thermalColor, CColor::White(), static_cast< float >(scale) / 255.f);
  const CColor lerpedColorCopyA = lerpedColor;
  const CColor lerpedColorCopyB = lerpedColorCopyA;
  GXSetTevColor(GX_TEVREG0, lerpedColorCopyB.GetGXColor());
  GXSetTevColor(GX_TEVREG1, tr1Clr);
  GXSetTevColor(GX_TEVREG2, tr2Clr);

  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_C2);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_A2);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);

  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(1);
  CGX::SetZMode(false, GX_ALWAYS, false);

  static const GXVtxDescList vtxDescrs[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

  const CTransform4f backupViewMatrix(CGraphics::mViewMatrix);
  CGraphics::CProjectionState backupProjectionState = CGraphics::GetProjectionState();

  CGraphics::SetOrtho(0.f, static_cast< float >(width), 0.f, static_cast< float >(height), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  GXPixModeSync();

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.5f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.5f, static_cast< float >(height));
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(static_cast< float >(width), 0.5f, static_cast< float >(height));
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(static_cast< float >(width), 0.5f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  GXSetDstAlpha(false, 0xff);
  CGraphics::SetProjectionState(backupProjectionState);
  CGraphics::SetViewPointMatrix(backupViewMatrix);

  CElementGen::SetMoveRedToAlphaBuffer(true);
  CDecal::SetMoveRedToAlphaBuffer(true);
}

void CCubeRenderer::DoThermalBlendHot() {
  GXSetAlphaUpdate(false);
  GXSetDstAlpha(true, 0);

  const CViewport& vp = CGraphics::mViewport;
  const int left = vp.mLeft;
  const int top = vp.mTop;
  const int width = vp.mWidth;
  const int height = vp.mHeight;
  void* dest = CGraphics::mpSpareBuffer;

  CGX::SetZMode(true, GX_LEQUAL, true);
  GXSetTexCopySrc(static_cast< u16 >(left), static_cast< u16 >(top), static_cast< u16 >(width),
                  static_cast< u16 >(height));
  GXSetTexCopyDst(static_cast< u16 >(width), static_cast< u16 >(height), GX_TF_I4, false);
  GXCopyTex(dest, false);

  x288_thermalPalette.Load();
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_C4, GX_TLUT0, 0,
                                     CGraphics::kSpareBufferTexMapID);

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetZMode(false, GX_LEQUAL, false);

  static const GXVtxDescList vtxDesc[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_CLEAR);

  const CTransform4f backupViewMtx(CGraphics::mViewMatrix);
  CGraphics::CProjectionState backupProjectionState = CGraphics::GetProjectionState();
  CGraphics::SetOrtho(0.f, static_cast< float >(width), 0.f, static_cast< float >(height), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  GXPixModeSync();

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.5f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.5f, static_cast< float >(height));
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(static_cast< float >(width), 0.5f, static_cast< float >(height));
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(static_cast< float >(width), 0.5f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  GXSetAlphaUpdate(true);
  CGraphics::SetProjectionState(backupProjectionState);
  CGraphics::SetViewPointMatrix(backupViewMtx);

  CElementGen::SetMoveRedToAlphaBuffer(false);
  CDecal::SetMoveRedToAlphaBuffer(false);
}

void CCubeRenderer::RenderFogVolume(const CColor& color, const CAABox& aabb,
                                    const TLockedToken< CModel >* model,
                                    const CSkinnedModel* skinnedModel) {
  if (!x318_28_disableFog) {
    x2ac_fogVolumes.push_back(
        CFogVolumeListItem(CGraphics::mModelMatrix, color, aabb, model, skinnedModel));
  }
}

void CCubeRenderer::ReallyRenderFogVolume(const CColor& color, const CAABox& aabb,
                                          const CModel* model, const CSkinnedModel* skinnedModel) {
  static const int skEdges[12][2] = {
      {0, 1}, {1, 3}, {3, 2}, {2, 0}, {4, 5}, {5, 7},
      {7, 6}, {6, 4}, {0, 4}, {1, 5}, {3, 7}, {2, 6},
  };
  static const GXVtxDescList vtxDescrs[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };

  void* copyBufA = CGraphics::mpSpareBuffer;
  void* copyBufB = static_cast< uchar* >(CGraphics::mpSpareBuffer) + 0x23000;

  const int vpLeft = CGraphics::mViewport.mLeft;
  const int vpTop = CGraphics::mViewport.mTop;
  const int vpWidth = CGraphics::mViewport.mWidth;
  const int vpHeight = CGraphics::mViewport.mHeight;

  int maxChunkW = 0x140;
  int maxChunkH = 0xe0;
  int chunkH = maxChunkH;
  int chunkW = maxChunkW;
  int drawLeft = 0;
  int drawTop = 0;
  bool recalcChunk = true;

  float uv0MinX = 0.f;
  float uv0MinY = 0.f;
  float uv0MaxX = 0.f;
  float uv0MaxY = 0.f;

  const CTransform4f modelXf(CGraphics::mModelMatrix);
  const CTransform4f viewXf(CGraphics::mViewMatrix);
  const CMatrix4f projMtx = CGraphics::GetPerspectiveProjectionMatrix();

  CVector2i minBounds(vpWidth, vpHeight);
  CVector2i maxBounds(0, 0);

  int& minY = minBounds[1];
  bool allOutside = true;
  rstl::reserved_vector< CVector3f, 8 > clipPts;
  rstl::reserved_vector< float, 8 > clipWs;
  for (int i = 0; i < 8; ++i) {
    const CVector3f worldPoint = modelXf * aabb.GetPoint(i);

    const CVector3f rotated = viewXf.TransposeRotate(CVector3f(worldPoint.GetX() - viewXf.Get03(),
                                                               worldPoint.GetY() - viewXf.Get13(),
                                                               worldPoint.GetZ() - viewXf.Get23()));

    const CVector3f projected = projMtx * rotated;
    clipPts.push_back(projected);
    clipWs.push_back(projMtx.MultiplyGetW(rotated));
  }

  for (int i = 0; i < 20; ++i) {
    CVector3f ndc;

    if (i < 8) {
      ndc = clipPts[i] / clipWs[i];
    } else {
      const int edge = i - 8;
      const int idxA = skEdges[edge][0];
      const int idxB = skEdges[edge][1];

      const float wA = clipWs[idxA];
      const float wB = clipWs[idxB];
      const CVector3f pA = clipPts[idxA];
      const CVector3f pB = clipPts[idxB];

      if ((pA.GetZ() / wA > 1.f) != (pB.GetZ() / wB > 1.f)) {
        const float t = -(wA - 1.f) / (wB - wA);
        if (t > 0.f && t < 1.f) {
          const float invW = 1.f / (wA + t * (wB - wA));
          ndc = invW * (pA + t * (pB - pA));
        } else {
          continue;
        }
      } else {
        continue;
      }
    }

    if (!(ndc.GetZ() <= 1.001f)) {
      continue;
    }

    const int scrX = static_cast< int >(static_cast< float >(vpWidth) * ndc.GetX() * 0.5f +
                                        static_cast< float >(vpWidth / 2));
    const int scrY = static_cast< int >(static_cast< float >(vpHeight) * -ndc.GetY() * 0.5f +
                                        static_cast< float >(vpHeight / 2));

    const int p0 = rstl::max_val(scrX, 0) & ~3;
    const int p1 = rstl::max_val(scrY, 0) & ~3;
    const int p2 = rstl::min_val(scrX + 3, vpWidth - 4) & ~3;
    const int p3 = rstl::min_val(scrY + 3, vpHeight - 4) & ~3;

    minBounds[0] = rstl::min_val(minBounds[0], p0);
    minY = rstl::min_val(minY, p1);
    maxBounds[0] = rstl::max_val(maxBounds[0], p2);
    maxBounds[1] = rstl::max_val(maxBounds[1], p3);

    allOutside = false;
  }

  int drawRight = vpWidth;
  int drawBottom = vpHeight;
  if (!allOutside) {
    maxChunkW = rstl::min_val(maxBounds.GetX() - minBounds.GetX(), maxChunkW);
    maxChunkH = rstl::min_val(maxBounds.GetY() - minBounds.GetY(), maxChunkH);

    drawRight = rstl::min_val(maxBounds[0], vpWidth);
    drawBottom = rstl::min_val(maxBounds[1], vpHeight);
    drawLeft = minBounds.GetX();
    drawTop = minBounds.GetY();
  }

  if (maxChunkW <= 0 || maxChunkH <= 0) {
    return;
  }

  if (((drawTop + vpTop) & 1) != 0) {
    --drawTop;
  }
  if (((drawLeft + vpLeft) & 1) != 0) {
    --drawLeft;
  }

  bool oldVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);

  const float texOffsetX = 0.5f / static_cast< float >(x1b8_fogVolumeRamp.GetWidth());
  const float texOffsetY = 0.5f / static_cast< float >(x1b8_fogVolumeRamp.GetHeight());
  float fogTexMtx[2][4] = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
  };
  fogTexMtx[0][3] = texOffsetX;
  fogTexMtx[1][3] = texOffsetY;
  GXLoadTexMtxImm(fogTexMtx, GX_TEXMTX0, GX_MTX2x4);

  const CAABox modelAabb(modelXf * aabb.GetMinPoint() - CVector3f(1.f, 1.f, 1.f),
                         modelXf * aabb.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f));

  bool doDoublePass = modelAabb.PointInside(CGraphics::mViewMatrix.GetTranslation()) &&
                      (model != nullptr || skinnedModel != nullptr);
  if (doDoublePass) {
    x318_26_requestRGBA6 = true;
    if (!x318_27_currentRGBA6) {
      doDoublePass = false;
    }
  }

  CGX::SetIndTexMtxSTPointFive(GX_ITM_0, 1);
  const int passCount = static_cast< int >(doDoublePass) + 1;

  for (int y = drawTop; y < drawBottom; y += chunkH) {
    if (drawBottom - y < chunkH) {
      chunkH = drawBottom - y;
      recalcChunk = true;
    }
    if (chunkW != maxChunkW) {
      chunkW = maxChunkW;
      recalcChunk = true;
    }

    const int copyTop = y + vpTop;

    for (int x = drawLeft; x < drawRight; x += chunkW) {
      CGraphics::SetModelMatrix(modelXf);

      if (drawRight - x < chunkW) {
        chunkW = drawRight - x;
        recalcChunk = true;
      }

      if (recalcChunk) {
        uv0MaxX = static_cast< float >(chunkW - 1) / static_cast< float >(chunkW);
        uv0MaxY = static_cast< float >(chunkH - 1) / static_cast< float >(chunkH);

        GXSetTexCopyDst(static_cast< u16 >(chunkW), static_cast< u16 >(chunkH), GX_CTF_A8,
                        GX_FALSE);

        uv0MinX = 0.5f / static_cast< float >(chunkW);
        uv0MinY = 0.5f / static_cast< float >(chunkH);
      }

      const int copyLeft = x + vpLeft;

      GXSetTexCopySrc(static_cast< u16 >(copyLeft), static_cast< u16 >(copyTop),
                      static_cast< u16 >(chunkW), static_cast< u16 >(chunkH));
      GXSetScissor(copyLeft, copyTop, chunkW, chunkH);

      CGX::SetZMode(true, GX_LEQUAL, true);
      CGX::SetNumTevStages(1);
      CGX::SetNumTexGens(1);
      CGX::SetNumChans(0);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
      x1b8_fogVolumeRamp.Load(GX_TEXMAP2, CTexture::kCM_Clamp);
      GXSetCullMode(GX_CULL_BACK);
      GXSetDstAlpha(GX_TRUE, 0xff);
      draw_box_or_model(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(false, GX_ALWAYS, false);
        draw_box_or_model(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      GXSetDstAlpha(GX_TRUE, 0);
      GXCopyTex(copyBufA, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufA, GX_TEXMAP0);

      GXSetCullMode(GX_CULL_FRONT);
      draw_box_or_model(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(true, GX_GREATER, false);
        draw_box_or_model(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      GXCopyTex(copyBufB, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufB, GX_TEXMAP1);

      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
      CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
      GXInvalidateTexAll();

      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
      CGX::SetNumTexGens(2);
      CGX::SetNumChans(0);

      const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
      CGX::SetVtxDescv(vtxDescrs);
      const CTransform4f oldView(CGraphics::mViewMatrix);

      CGraphics::SetOrtho(0.f, static_cast< float >(vpWidth), 0.f, static_cast< float >(vpHeight),
                          -4096.f, 4096.f);
      CGraphics::SetViewPointMatrix(CTransform4f::Identity());
      CGraphics::SetModelMatrix(CTransform4f::Identity());

      CGX::SetZMode(false, GX_ALWAYS, false);
      GXSetCullMode(GX_CULL_NONE);
      GXSetAlphaUpdate(GX_FALSE);

      const int right = x + chunkW;
      const int bottom = y + chunkH;
      for (int pass = 0; pass < passCount; ++pass) {
        if (pass == 0) {
          CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0,
                              GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
          CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_ITF_8, GX_ITB_NONE, GX_ITM_0,
                              GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
          GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
          GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD0, GX_TEXMAP0);
          CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

          CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
          CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
          CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
          CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);

          CGX::SetNumIndStages(2);
          CGX::SetNumTevStages(2);
          CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
        } else if (pass == 1) {
          CGX::SetTevDirect(GX_TEVSTAGE1);
          GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);

          CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

          CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_APREV, GX_CC_CPREV, GX_CC_ZERO);
          CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
          CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
          CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);

          CGX::SetNumIndStages(1);
          CGX::SetNumTevStages(2);
          CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_CLEAR);
        }

        CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

        GXPosition3f32(static_cast< float >(x), 0.5f, static_cast< float >(y));
        GXTexCoord2f32(uv0MinX, uv0MinY);

        GXPosition3f32(static_cast< float >(x), 0.5f, static_cast< float >(bottom));
        GXTexCoord2f32(uv0MinX, (uv0MinY + uv0MaxY));

        GXPosition3f32(static_cast< float >(right), 0.5f, static_cast< float >(bottom));
        GXTexCoord2f32((uv0MinX + uv0MaxX), (uv0MinY + uv0MaxY));

        GXPosition3f32(static_cast< float >(right), 0.5f, static_cast< float >(y));
        GXTexCoord2f32((uv0MinX + uv0MaxX), uv0MinY);

        CGX::End();
      }

      GXSetAlphaUpdate(GX_TRUE);
      CGraphics::SetViewPointMatrix(oldView);
      CGX::SetNumIndStages(0);
      CGX::SetTevDirect(GX_TEVSTAGE0);
      CGX::SetTevDirect(GX_TEVSTAGE1);
      GXSetCullMode(GX_CULL_FRONT);
      CGraphics::SetProjectionState(oldProjection);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    }
  }

  GXSetScissor(vpLeft, vpTop, vpWidth, vpHeight);
  CGraphics::SetUseVideoFilter(oldVideoFilter);
}

void CCubeRenderer::SetThermal(bool thermal, float level, const CColor& color) {
  x318_29_thermalVisor = thermal;
  x2f0_thermalVisorLevel = level;
  x2f4_thermalColor = color;
  CDecal::SetMoveRedToAlphaBuffer(false);
  CElementGen::SetMoveRedToAlphaBuffer(false);
}

void CCubeRenderer::EnablePVS(const CPVSVisSet* set, int areaIdx) {
  xc8_pvsVisSet = *set;
  xe0_pvsAreaIdx = areaIdx;
}

void CCubeRenderer::DisablePVS() { xc8_pvsVisSet = rstl::optional_object_null(); }

namespace {
struct fog_sorter {
  bool operator()(const CCubeRenderer::CFogVolumeListItem& xfA,
                  const CCubeRenderer::CFogVolumeListItem& xfB) const {
    const CTransform4f& viewXf = CGraphics::GetViewMatrix();
    const CVector3f pos = viewXf.GetTranslation();

    const CAABox boxA = xfA.x34_aabb.GetTransformedAABox(xfA.x0_xf);
    const CAABox boxB = xfB.x34_aabb.GetTransformedAABox(xfB.x0_xf);

    bool insideA = boxA.PointInside(CVector3f(pos.GetX(), pos.GetY(), boxA.GetMinPoint().GetZ()));
    bool insideB = boxB.PointInside(CVector3f(pos.GetX(), pos.GetY(), boxB.GetMinPoint().GetZ()));
    if (insideA != insideB) {
      return insideA;
    }

    const CVector3f viewDir = viewXf.GetColumn(kDY);
    const float dotA = CVector3f::Dot(viewDir, boxA.FurthestPointAlongVector(viewDir));
    const float dotB = CVector3f::Dot(viewDir, boxB.FurthestPointAlongVector(viewDir));
    return dotA < dotB;
  }
};
} // namespace

void CCubeRenderer::PostRenderFogs() {
  for (AUTO(warpIt, x2c4_spaceWarps.begin()); warpIt != x2c4_spaceWarps.end(); ++warpIt) {
    _DrawSpaceWarp(warpIt->first, warpIt->second);
  }
  x2c4_spaceWarps.clear();

  x2ac_fogVolumes.sort(fog_sorter());

  for (AUTO(fogIt, x2ac_fogVolumes.begin()); fogIt != x2ac_fogVolumes.end(); ++fogIt) {
    CFogVolumeListItem& fog = *fogIt;
    CGraphics::SetModelMatrix(fog.x0_xf);
    ReallyRenderFogVolume(fog.x30_color, fog.x34_aabb, fog.x4c_model ? **fog.x4c_model : nullptr,
                          fog.x5c_skinnedModel);
  }

  x2ac_fogVolumes.clear();
}

void CCubeRenderer::DoThermalModelDraw(const CCubeModel& model, const CColor& mulCol,
                                       const CColor& addCol, const float* pos, const float* nrm,
                                       const CModelFlags& flags) {
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, true, GX_PTTEXMTX0);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  x220_sphereRamp.Load(GX_TEXMAP0, CTexture::kCM_Clamp);

  CTransform4f texXf =
      CGraphics::mViewMatrix.GetQuickInverse().MultiplyIgnoreTranslation(CGraphics::mModelMatrix);
  texXf.SetTranslation(CVector3f::Zero());

  GXLoadTexMtxImm(const_cast< MtxPtr >(texXf.GetCStyleMatrix()), GX_TEXMTX0, GX_MTX3x4);
  static const float postMtx[3][4] = {
      {0.5f, 0.f, 0.f, 0.5f},
      {0.f, 0.f, 0.5f, 0.5f},
      {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(const_cast< MtxPtr >(postMtx), GX_PTTEXMTX0, GX_MTX3x4);

  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_KONST);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetTevKColor(GX_KCOLOR0, addCol.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  GXSetTevColor(GX_TEVREG0, mulCol.GetGXColor());

  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);

  CGX::SetZMode(
      (flags.GetOtherFlags() & CModelFlags::kF_DepthCompare) == CModelFlags::kF_DepthCompare,
      GX_LEQUAL,
      (flags.GetOtherFlags() & CModelFlags::kF_DepthUpdate) == CModelFlags::kF_DepthUpdate);

  const ushort drawFlags = flags.GetOtherFlags();
  model.DrawFlat(pos, nrm,
                 (drawFlags & CModelFlags::kF_ThermalUnsortedOnly) != 0 ? kSS_Unsorted : kSS_All);
}

void CCubeRenderer::DrawThermalModel(const CModel& model, const CColor& mulCol,
                                     const CColor& addCol, const float* pos, const float* nrm,
                                     const CModelFlags& flags) {
  const CCubeModel* modelInst = model.GetCubeModel();
  model.UpdateLastFrame();
  DoThermalModelDraw(*modelInst, mulCol, addCol, pos, nrm, flags);
}

void CCubeRenderer::DrawModelDisintegrate(const CModel& model, const CTexture& tex,
                                          const CColor& color, const float* pos, const float* nrm,
                                          float t) {

  tex.Load(GX_TEXMAP0, CTexture::kCM_Clamp);
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
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());

  const CAABox& modelBounds = model.GetCubeModel()->GetBoundingBox();

  const CRelAngle rotateAng(-0.7853982f);
  CTransform4f rotateXf = CTransform4f::RotateX(rotateAng);

  const CAABox xfdAabb = modelBounds.GetTransformedAABox(rotateXf);

  const CVector3f& aabbMin = xfdAabb.GetMinPoint();
  const CVector3f& aabbMax = xfdAabb.GetMaxPoint();

  CVector3f xlateVec(-aabbMin.GetX(), -aabbMin.GetY(), -aabbMin.GetZ());
  const CVector3f dimensions = aabbMax - aabbMin;

  rotateXf = (CTransform4f::Scale(5.f / dimensions.GetX(), 5.f / dimensions.GetY(),
                                  5.f / dimensions.GetZ()) *
              CTransform4f::Translate(xlateVec)) *
             rotateXf;

  const CAABox transformedAabb =
      model.GetCubeModel()->GetBoundingBox().GetTransformedAABox(rotateXf);
  (void)transformedAabb;

  const float f1 = -(1.f - t) * 6.f + 1.f;
  const float f2 = -0.85f * t - 0.15f;
  float ptTex0[3][4] = {
      {1.f, 1.f, 0.f, t},
      {0.f, 0.f, 1.f, f1},
      {0.f, 0.f, 0.f, 1.f},
  };
  float ptTex1[3][4] = {
      {1.f, 1.f, 0.f, f2},
      {0.f, 0.f, 1.f, f1},
      {0.f, 0.f, 0.f, 1.f},
  };

  GXLoadTexMtxImm(const_cast< MtxPtr >(rotateXf.GetCStyleMatrix()), GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(ptTex0, GX_PTTEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(ptTex1, GX_PTTEXMTX1, GX_MTX3x4);

  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX1);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, true);

  model.UpdateLastFrame();
  model.GetCubeModel()->DrawFlat(pos, nrm, kSS_All);

  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void CCubeRenderer::DrawModelFlat(const CModel& model, const CModelFlags& flags, const bool unsortedOnly,
                                  const float* pos, const float* nrm) {
  const char blendMode = static_cast< char >(flags.GetBlendMode());
  if (blendMode > 6) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else if (blendMode > 4) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }

  const uint otherFlags = flags.GetOtherFlags();
  CGX::SetZMode(true, (otherFlags & CModelFlags::kF_DepthCompare) != 0 ? GX_LEQUAL : GX_ALWAYS,
                (otherFlags & CModelFlags::kF_DepthUpdate) != 0);

  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(0);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKColor(GX_KCOLOR0, flags.GetColorRef().GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);

  model.UpdateLastFrame();
  const CCubeModel* modelInst = model.GetCubeModel();
  modelInst->DrawFlat(pos, nrm, unsortedOnly ? kSS_Unsorted : kSS_All);
}

void CCubeRenderer::DrawAreaGeometry(int areaIdx, uint mask, uint targetMask) {
  x318_30_inAreaDraw = true;
  SetupRendererStates(true);

  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    CAreaListItem& item = *it;
    if (areaIdx != -1 && areaIdx != item.GetAreaId()) {
      continue;
    }

    rstl::vector< rstl::auto_ptr< CCubeModel > >& models = *item.GetModelList();

    const CPVSVisSet* pvs = 0;
    if (xc8_pvsVisSet) {
      pvs = xc8_pvsVisSet.get_ptr();
    } else {
      pvs = 0;
    }
    if (xe0_pvsAreaIdx != item.GetAreaId()) {
      pvs = 0;
    }

    int modelIdx = 0;
    for (AUTO(modelIt, models.begin()); modelIt != models.end(); ++modelIt, ++modelIdx) {
      CCubeModel& model = **modelIt;

      if (pvs != 0) {
        const bool visible = pvs->GetVisible(modelIdx) != kVSS_EndOfTree;
        if ((xc4_pvsState == 1 && !visible) || (xc4_pvsState == 2 && visible)) {
          continue;
        }
      }

      if (targetMask != (static_cast< uint >(mask) & model.GetModelFlags())) {
        continue;
      }

      if (!x44_frustumPlanes.BoxInFrustumPlanes(model.GetBoundingBox())) {
        continue;
      }

      if (targetMask != (static_cast< uint >(mask) & model.GetModelFlags())) {
        continue;
      }

      model.SetArraysCurrent();

      for (CCubeSurface surf = model.GetNormalSurfaces(); surf.IsValid();
           surf = surf.GetNextSurface()) {
        model.DrawSurface(surf, skNormalFlag);
      }

      for (CCubeSurface surf = model.GetAlphaSurfaces(); surf.IsValid();
           surf = surf.GetNextSurface()) {
        model.DrawSurface(surf, skNormalFlag);
      }
    }
  }

  SetupCGraphicsStates();
  x318_30_inAreaDraw = false;
}

void CCubeRenderer::SetThermalColdScale(float scale) {
  const float clampedScale = CMath::Clamp(0.f, scale, 1.f);
  x2f8_thermalColdScale = CCast::ToUint8(clampedScale * 255.f);
}

void CCubeRenderer::SetGXRegister1Color(const CColor& color) {
  GXSetTevColor(GX_TEVREG1, color.GetGXColor());
}

void CCubeRenderer::SetWorldLightFadeLevel(float level) {
  const uchar val = CCast::ToUint8(level * 255.f);
  x2fc_tevReg1Color = CColor(val, val, val, 255);
}

CAABox CCubeRenderer::GetAreaModelBounds(int areaIdx, int modelIdx) {
  if (areaIdx != -1) {
    for (AUTO(areaIt, x1c_areaListItems.begin()); areaIt != x1c_areaListItems.end(); ++areaIt) {
      if (areaIdx != areaIt->GetAreaId()) {
        continue;
      }

      int idx = 0;
      const rstl::vector< rstl::auto_ptr< CCubeModel > >& models = *areaIt->GetModelList();
      for (AUTO(modelIt, models.begin()); modelIt != models.end(); ++modelIt, ++idx) {
        if (idx == modelIdx) {
          return modelIt->get()->GetBoundingBox();
        }
      }
    }
  }

  return CAABox::MakeNullBox();
}

void CCubeRenderer::PrepareDynamicLights(const rstl::vector< CLight >& lights) {
  if (lights.size() != x300_dynamicLights.size()) {
    x300_dynamicLights = rstl::vector< CLight >();
  }

  x300_dynamicLights = lights;

  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    OSGetTime();

    const CAreaRenderOctTree* octTree = it->x4_octTree;
    if (octTree != 0) {
      const uint octreeWordCount = octTree->x14_bitmapWordCount;
      rstl::vector< uint >& octWords = it->x1c_lightOctreeWords;
      octWords = rstl::vector< uint >();

      if (!lights.empty()) {
        octWords.resize(octreeWordCount * x300_dynamicLights.size(), 0);

        int wordOffset = 0;
        for (int i = 0; i < lights.size(); ++i) {
          const CLight& light = lights[i];
          const float radius = light.GetRadius();
          const CAABox aabb(light.GetPosition() - CVector3f(radius, radius, radius),
                            light.GetPosition() + CVector3f(radius, radius, radius));
          octTree->FindOverlappingModels(octWords.data() + wordOffset, aabb);
          wordOffset = wordOffset + octreeWordCount;
        }

        OSGetTime();
      }
    }
  }
}

void CCubeRenderer::FindOverlappingWorldModels(rstl::vector< uint >& modelBits,
                                               const CAABox& aabb) {
  int wordCount = 0;
  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    if (it->x4_octTree != 0) {
      wordCount += it->x4_octTree->x14_bitmapWordCount;
    }
  }

  if (wordCount == 0) {
    modelBits = rstl::vector< uint >();
    return;
  }

  if (wordCount != modelBits.capacity()) {
    modelBits = rstl::vector< uint >();
  } else {
    modelBits.clear();
  }

  modelBits.resize(wordCount);

  int curWord = 0;
  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    const CAreaRenderOctTree* octTree = it->x4_octTree;
    if (octTree == nullptr) {
      continue;
    }

    octTree->FindOverlappingModels(modelBits.data() + curWord, aabb);

    for (int i = 0, wordModel = 0; i < octTree->x14_bitmapWordCount; ++i, wordModel += 0x20) {
      uint& word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }

      for (int j = 0; j < 0x20; ++j) {
        const uint mask = (1 << j);
        if ((word & mask) != 0 &&
            !it->GetModelList()->at(wordModel + j)->GetBoundingBox().DoBoundsOverlap(aabb)) {
          word &= ~mask;
        }
      }
    }

    curWord += octTree->x14_bitmapWordCount;
  }
}

int CCubeRenderer::DrawOverlappingWorldModelIDs(int alphaVal, rstl::vector< uint >& modelBits,
                                                const CAABox& aabb, int, int) {
  SetupRendererStates(true);

  const CModelFlags flags = CModelFlags::Normal();

  int curWord = 0;
  int alpha = alphaVal;

  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    const CAreaRenderOctTree* octTree = it->x4_octTree;
    if (octTree == nullptr) {
      continue;
    }

    for (int i = 0, wordModel = 0; i < octTree->x14_bitmapWordCount; ++i, wordModel += 0x20) {
      const uint word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }

      for (int j = 0; j < 0x20; ++j) {
        if ((word & (1 << j)) == 0) {
          continue;
        }

        CCubeModel* model = it->GetModelList()->at(wordModel + j).get();
        GXSetDstAlpha(GX_ENABLE, static_cast< u8 >(alpha * 4));
        CCubeMaterial::KillCachedViewDepState();
        model->SetArraysCurrent();

        for (CCubeSurface surf = model->GetNormalSurfaces(); surf.IsValid();
             surf = surf.GetNextSurface()) {
          if (surf.GetBounds().DoBoundsOverlap(aabb)) {
            model->DrawSurface(surf, flags);
          }
        }

        ++alpha;
        if (alpha >= 0x40) {
          SetupCGraphicsStates();
          return alpha;
        }
      }
    }

    curWord += octTree->x14_bitmapWordCount;
  }

  SetupCGraphicsStates();
  return alpha;
}

void CCubeRenderer::DrawOverlappingWorldModelShadows(int alphaVal, rstl::vector< uint >& modelBits,
                                                     const CAABox& aabb, int, int) {
  const GXColor black = {0, 0, 0, 0};
  GXColor color = black;

  int curWord = 0;
  int alpha = alphaVal;

  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    const CAreaRenderOctTree* octTree = it->x4_octTree;
    if (octTree == 0) {
      continue;
    }

    for (int i = 0, wordModel = 0; i < octTree->x14_bitmapWordCount; ++i, wordModel += 0x20) {
      const uint word = modelBits[curWord + i];
      if (word == 0) {
        continue;
      }

      for (int j = 0; j < 0x20; ++j) {
        if ((word & (1 << j)) == 0) {
          continue;
        }

        CCubeModel* model = it->GetModelList()->at(wordModel + j).get();
        color.a = static_cast< u8 >(alpha * 4);
        CGX::SetTevKColor(GX_KCOLOR0, color);
        model->SetArraysCurrent();

        for (CCubeSurface surf = model->GetNormalSurfaces(); surf.IsValid();
             surf = surf.GetNextSurface()) {
          if (surf.GetBounds().DoBoundsOverlap(aabb)) {
            const CCubeMaterial mat = model->GetMaterialByIndex(surf.GetMaterialIndex());
            CGX::SetVtxDescv_Compressed(mat.GetVertexDescLwzx());
            CGX::CallDisplayList(surf.GetDisplayList(), surf.GetDisplayListSize());
          }
        }

        ++alpha;
        if (alpha >= 0x40) {
          return;
        }
      }
    }

    curWord += octTree->x14_bitmapWordCount;
  }
}

void* CCubeRenderer::GetRenderToTexBuffer(int idx) {
  return static_cast< uchar* >(CGraphics::mpSpareBuffer) +
         (static_cast< uint >(idx * CGraphics::mSpareBufferSize) >> 4);
}

void CCubeRenderer::CopyTex(const int div, const bool half, void* dest, const GXTexFmt fmt, const bool clear) {
  const CViewport& vp = CGraphics::mViewport;
  uint width = vp.mWidth;
  uint height = vp.mHeight;
  const uint copyHeight = height / div;

  GXSetTexCopySrc(static_cast< u16 >(vp.mLeft), static_cast< u16 >(vp.mTop + height - copyHeight),
                  static_cast< u16 >(width / div), static_cast< u16 >(copyHeight));

  const uint destWidth = (half ? static_cast< int >(width) / 2 : width) / div;
  const uint destHeight = (half ? static_cast< int >(height) / 2 : height) / div;
  GXSetTexCopyDst(static_cast< u16 >(destWidth), static_cast< u16 >(destHeight), fmt, half);

  CColor clearColor(0);
  CGraphics::SetClearColor(clearColor);

  GXSetColorUpdate(false);
  GXCopyTex(dest ? dest : CGraphics::mpSpareBuffer, clear);
  GXSetColorUpdate(true);
  GXPixModeSync();
}

void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float blurRadius, float blurRadiusB) {
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;

  CGraphics::SetOrtho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetDepthWriteMode(false, kE_GEqual, false);

  CopyTex(1, true, GetRenderToTexBuffer(8), GX_TF_A8, true);
  GXSetDstAlpha(GX_TRUE, 0);

  CGraphics::LoadDolphinSpareTexture(width / 2, height / 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const GXVtxDescList vtxDescrs[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);

  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_RASC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  GXSetColorUpdate(GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanMatColor(CGX::Channel0, CColor::White().GetGXColor());
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());

  const uint white = CColor::White().GetColor_u32();

  GXSetDstAlpha(GX_FALSE, 0);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(white);
  GXTexCoord2f32(0.f, 1.f);

  GXPosition3f32(0.5f, 0.f, 0.f);
  GXColor1u32(white);
  GXTexCoord2f32(1.f, 1.f);

  GXPosition3f32(0.f, 0.f, 0.5f);
  GXColor1u32(white);
  GXTexCoord2f32(0.f, 0.f);

  GXPosition3f32(0.5f, 0.f, 0.5f);
  GXColor1u32(white);
  GXTexCoord2f32(1.f, 0.f);

  CGX::End();

  CopyTex(2, true, GetRenderToTexBuffer(8), GX_TF_A8, true);
  GXSetColorUpdate(GX_FALSE);

  CGraphics::LoadDolphinSpareTexture(width >> 2, height >> 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurOffsets[8][2] = {
      {-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f},
      {-1.f, 0.f},  {1.f, 0.f},  {0.f, 1.f},  {0.f, -1.f},
  };

  const float blurScale = blurRadius * (2.f / static_cast< float >(width));
  const uint blurColorA = CColor(1.f, 1.f, 1.f, 0.3f).GetColor_u32();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    CVector2f ofs(blurScale * blurOffsets[i][0], blurScale * blurOffsets[i][1]);
    float& offsetX = ofs[0];

    GXPosition3f32(offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(offsetX + 0.25f, 0.f, ofs[1]);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(offsetX, 0.f, ofs[1] + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(offsetX + 0.25f, 0.f, ofs[1] + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }

  GXSetDstAlpha(GX_FALSE, 0);
  CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  const uint whiteColor = CColor(1.f, 1.f, 1.f, 1.f).GetColor_u32();

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(0.f, 1.f);

  GXPosition3f32(0.25f, 0.f, 0.f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(1.f, 1.f);

  GXPosition3f32(0.f, 0.f, 0.25f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(0.f, 0.f);

  GXPosition3f32(0.25f, 0.f, 0.25f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(1.f, 0.f);

  CGX::End();

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

  CopyTex(4, false, GetRenderToTexBuffer(8), GX_TF_A8, true);
  GXSetDstAlpha(GX_FALSE, 0);
  GXSetColorUpdate(GX_FALSE);

  CGraphics::LoadDolphinSpareTexture(width >> 2, height >> 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurScaleB = blurRadiusB * (1.5f / static_cast< float >(width));
  const uint blurColorB = CColor(1.f, 1.f, 1.f, 0.35f).GetColor_u32();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    CVector2f ofs(blurScaleB * blurOffsets[i][0], blurScaleB * blurOffsets[i][1]);
    float& offsetX = ofs[0];

    GXPosition3f32(offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(0.25f + offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(offsetX, 0.f, 0.25f + ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(0.25f + offsetX, 0.f, 0.25f + ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }
}

void CCubeRenderer::ReallyDrawPhazonSuitEffect(const CColor& modColor, const CTexture& maskTex) {
  maskTex.Load(CGraphics::kSpareBufferTexMapID, CTexture::kCM_Repeat);

  const GXVtxDescList vtxDescrs[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);

  IRenderer* renderer = this;
  renderer->SetBlendMode_AdditiveAlpha();

  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_RASC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);

  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());

  GXSetDstAlpha(GX_TRUE, 0);
  GXSetColorUpdate(GX_TRUE);

  const uint color = modColor.GetColor_u32();

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(color);
  GXTexCoord2f32(0.f, 1.f);

  GXPosition3f32(1.f, 0.f, 0.f);
  GXColor1u32(color);
  GXTexCoord2f32(1.f, 1.f);

  GXPosition3f32(0.f, 0.f, 1.f);
  GXColor1u32(color);
  GXTexCoord2f32(0.f, 0.f);

  GXPosition3f32(1.f, 0.f, 1.f);
  GXColor1u32(color);
  GXTexCoord2f32(1.f, 0.f);

  CGX::End();

  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

void CCubeRenderer::ReallyDrawPhazonSuitIndirectEffect(const CColor& vertColor,
                                                       const CTexture& maskTex,
                                                       const CTexture& indirectTex,
                                                       const CColor& modColor, float scale,
                                                       float offX, float offY) {

  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;

  CVector2i topLeft(0, 0);
  CVector2i bottomRight(width, height);
  CVector2f uv0Min(0.f, 0.f);
  float& uv0MinX = uv0Min[0];
  CVector2f uv0Max(1.f, 1.f);
  float& uv0MaxX = uv0Max[0];

  CVector2i dim = bottomRight - topLeft;
  CVector2i halfDim = dim / 2;
  int& halfX = halfDim[0];

  if (dim.GetX() <= 0 || dim.GetY() <= 0) {
    return;
  }

  CGraphics::LoadDolphinSpareTexture(halfX, halfDim.GetY(), GX_TF_RGB565, CGraphics::mpSpareBuffer,
                                     CGraphics::kSpareBufferTexMapID);
  indirectTex.Load(GX_TEXMAP1, CTexture::kCM_Repeat);
  maskTex.Load(GX_TEXMAP2, CTexture::kCM_Repeat);

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  const CColor kColor(modColor.GetRed() * modColor.GetAlpha(),
                      modColor.GetGreen() * modColor.GetAlpha(),
                      modColor.GetBlue() * modColor.GetAlpha(), 0.25f * modColor.GetAlpha());
  CGX::SetTevKColor(GX_KCOLOR0, kColor.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);

  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0);

  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);

  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());

  float indScale = scale;
  u8 scaleExp = 1;
  while (CMath::AbsF(indScale) >= 0.99f) {
    indScale *= 0.5f;
    ++scaleExp;
  }
  while (CMath::AbsF(indScale) < 0.49f) {
    indScale *= 2.f;
    --scaleExp;
  }

  float indMtx[2][3] = {
      {indScale, 0.f, offX * indScale},
      {0.f, indScale, offY * indScale},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, scaleExp);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);

  CGX::SetNumIndStages(1);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(3);
  CGX::SetNumChans(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

  static const GXVtxDescList vtxDesc[6] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);

  const CGraphics::CProjectionState backupProjection = CGraphics::GetProjectionState();
  const CTransform4f backupView(CGraphics::mViewMatrix);

  CGraphics::SetOrtho(0.f, static_cast< float >(width), 0.f, static_cast< float >(height), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);
  GXSetDstAlpha(GX_TRUE, 0);

  const uint vertColorU32 = vertColor.GetColor_u32();
  CVector2f uv1Min(0.f, 0.f);
  float& uv1MinX = uv1Min[0];
  CVector2f uv1Max(1.f, 1.f);
  float& uv1MaxX = uv1Max[0];

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(topLeft.GetX(), 0.5f, topLeft.GetY());
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.01f, 0.01f);
  GXTexCoord2f32(uv0MinX, uv0Min[1]);
  GXTexCoord2f32(uv1MinX, uv1Min[1]);

  GXPosition3f32(topLeft.GetX(), 0.5f, bottomRight.GetY());
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.01f, 0.99f);
  GXTexCoord2f32(uv0MinX, uv0Max[1]);
  GXTexCoord2f32(uv1MinX, uv1Max[1]);

  GXPosition3f32(bottomRight.GetX(), 0.5f, bottomRight.GetY());
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.99f, 0.99f);
  GXTexCoord2f32(uv0MaxX, uv0Max[1]);
  GXTexCoord2f32(uv1MaxX, uv1Max[1]);

  GXPosition3f32(bottomRight.GetX(), 0.5f, topLeft.GetY());
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.99f, 0.01f);
  GXTexCoord2f32(uv0MaxX, uv0Min[1]);
  GXTexCoord2f32(uv1MaxX, uv1Min[1]);

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProjection);
  CGraphics::SetViewPointMatrix(backupView);
}

void CCubeRenderer::DrawPhazonSuitIndirectEffect(
    const CColor& nonIndirectColor,
    const rstl::optional_object< TCachedToken< CTexture > >& indirectTex,
    float blurRadius, float scale, float offX, float offY, const CColor& indirectColor) {
  if (x318_27_currentRGBA6 && x310_phazonSuitMaskCountdown != 0) {
    const CTransform4f backupView(CGraphics::mViewMatrix);
    const CGraphics::CProjectionState backupProjection = CGraphics::GetProjectionState();

    if (x314_phazonSuitMask.null() ||
        x314_phazonSuitMask->GetWidth() != (CGraphics::GetViewport().mWidth >> 2) ||
        x314_phazonSuitMask->GetHeight() != (CGraphics::GetViewport().mHeight >> 2)) {
      return;
    }

    DoPhazonSuitIndirectAlphaBlur(blurRadius, blurRadius);
    x314_phazonSuitMask->SetFlag1(true);
    CopyTex(4, false, x314_phazonSuitMask->GetBitMapData(0), GX_TF_A8, true);

    CTexture* indTex = 0;
    if (indirectTex && (indTex = indirectTex->GetObject())) {
      ReallyDrawPhazonSuitIndirectEffect(CColor(1.f, 1.f, 1.f, 1.f), *x314_phazonSuitMask, *indTex,
                                         indirectColor, scale, offX, offY);
    } else {
      ReallyDrawPhazonSuitEffect(nonIndirectColor, *x314_phazonSuitMask);
    }

    x314_phazonSuitMask->UnLock();
    CGraphics::SetViewPointMatrix(backupView);
    CGraphics::SetProjectionState(backupProjection);
    x310_phazonSuitMaskCountdown = 2;
  }

  GXSetDstAlpha(GX_FALSE, 0);
}

void CCubeRenderer::AllocatePhazonSuitMaskTexture() {
  x318_26_requestRGBA6 = true;

  if (x314_phazonSuitMask.null()) {
    x314_phazonSuitMask = rs_new CTexture(kTF_I8, CGraphics::GetViewport().mWidth >> 2,
                                          CGraphics::GetViewport().mHeight >> 2, 1);
  }

  x310_phazonSuitMaskCountdown = 2;
}

void CCubeRenderer::DrawXRayOutline(const CAABox& bounds, const float*, const float*) {
  for (AUTO(it, x1c_areaListItems.begin()); it != x1c_areaListItems.end(); ++it) {
    const CAreaRenderOctTree* octTree = it->x4_octTree;
    if (octTree == 0) {
      continue;
    }

    rstl::vector< uint > modelBits;
    octTree->FindOverlappingModels(modelBits, bounds);

    for (int i = 0, wordModel = 0; i < octTree->x14_bitmapWordCount; ++i, wordModel += 0x20) {
      const uint word = modelBits[i];
      if (word == 0) {
        continue;
      }

      for (int j = 0; j < 0x20; ++j) {
        if ((word & (1 << j)) == 0) {
          continue;
        }

        CCubeModel* modelInst = it->GetModelList()->at(wordModel + j).get();
        modelInst->SetArraysCurrent();

        for (CCubeSurface surf = modelInst->GetNormalSurfaces(); surf.IsValid();
             surf = surf.GetNextSurface()) {
          if (surf.GetBounds().DoBoundsOverlap(bounds)) {
            const CCubeMaterial mat = modelInst->GetMaterialByIndex(surf.GetMaterialIndex());
            CGX::SetVtxDescv_Compressed(mat.GetVertexDescLwzx());
            CGX::CallDisplayList(surf.GetDisplayList(), surf.GetDisplayListSize());
          }
        }
      }
    }
  }
}

void CCubeRenderer::DrawString(const char* string, int x, int y) {
  x10_font.DrawString(string, x, y, CColor::White());
}
