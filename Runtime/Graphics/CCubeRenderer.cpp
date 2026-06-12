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
#include "Runtime/Logging.hpp"

#include <utility>

namespace metaforce {
constexpr int LogicalFbWidth = 640;
constexpr int LogicalFbHeight = 448;

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
    //    spdlog::fatal("Full bucket!!!");
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
    spdlog::fatal("Rendering buckets filled to capacity");
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
  float threshold = 14.f;
  float halfScale = 128.f;

  ushort* base = reinterpret_cast<ushort*>(x150_reflectionTex.Lock());
  ushort* data = base;
  for (int yBlock = 0; yBlock < 8; ++yBlock) {
    for (int xBlock = 0; xBlock < 8; ++xBlock) {
      for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
          float fx = 0.f;
          float fy = 0.f;
          zeus::CVector2f vec(static_cast<float>(xBlock * 4 + x - 14), static_cast<float>(yBlock * 4 + y - 14));
          float mag = vec.magnitude();
          if (mag <= threshold) {
            vec.normalize();
            vec *= (threshold - mag) / threshold;
            fx = vec.x();
            fy = vec.y();
          }

          float scaledX = halfScale * fx + halfScale;
          int ix = static_cast<int>(zeus::clamp(0.f, scaledX, 255.f));
          float scaledY = halfScale * fy + halfScale;
          int iy = static_cast<int>(zeus::clamp(0.f, scaledY, 255.f));

          *data++ = bswap16(static_cast<ushort>((iy & 0xFF) | ((ix & 0xFF) << 8)));
        }
      }
    }
  }
  x150_reflectionTex.UnLock();
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
  const int height = 32;
  const int width = 32;
  const float halfRes = (height - 1) / 2.f;

  u8* data = static_cast<u8*>(x220_sphereRamp.Lock());
  for (int y = 0; y < height; ++y) {
    int start = y * width;
    for (int x = 0; x < width; ++x) {
      // I8 block is 8x4 (WxH)
      // Convert swizzled coords to linear
      float fx = static_cast<float>(((y % 4) << 3) + (x & 7));
      float fy = static_cast<float>(((y / 4) << 2) + (x >> 3));
      fx = (fx / halfRes) - 1.f;
      fy = (fy / halfRes) - 1.f;
      float mag = sqrtf(fx * fx + fy * fy);
      float value = std::clamp(1.f - (mag * mag), 0.f, 1.f);
      data[start + x] = static_cast<u8>(value * 255.f);
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
  const int width = CGraphics::mViewport.mWidth;
  const int height = CGraphics::mViewport.mHeight;

  zeus::CVector2i topLeft(0, 0);
  zeus::CVector2i bottomRight(width, height);
  zeus::CVector2f uv0Min(0.f, 0.f);
  zeus::CVector2f uv0Max(1.f, 1.f);

  zeus::CVector2i dim = bottomRight - topLeft;
  if (dim.x <= 0 || dim.y <= 0) {
    return;
  }

  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth / 2, LogicalFbHeight / 2, GX_TF_RGB565, CGraphics::mpSpareBuffer,
                                     CGraphics::kSpareBufferTexMapID);
  indTex.Load(GX_TEXMAP1, EClampMode::Repeat);
  maskTex.Load(GX_TEXMAP2, EClampMode::Repeat);

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  const zeus::CColor kColor(modColor.r() * modColor.a(), modColor.g() * modColor.a(), modColor.b() * modColor.a(),
                            0.25f * modColor.a());
  CGX::SetTevKColor(GX_KCOLOR0, to_gx_color(kColor));
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

  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CGX::SetChanAmbColor(CGX::EChannelId::Channel0, to_gx_color(zeus::skBlack));

  float indScale = scale;
  u8 scaleExp = 1;
  while (fabsf(indScale) >= 0.99f) {
    indScale *= 0.5f;
    ++scaleExp;
  }
  while (fabsf(indScale) < 0.49f) {
    indScale *= 2.f;
    --scaleExp;
  }

  float indMtx[2][3] = {
      {indScale, 0.f, offX * indScale},
      {0.f, indScale, offY * indScale},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, scaleExp);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, false,
                      false, GX_ITBA_OFF);

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
  const zeus::CTransform backupView(CGraphics::mViewMatrix);

  CGraphics::SetOrtho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform());
  CGraphics::SetModelMatrix(zeus::CTransform());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);
  GXSetDstAlpha(GX_TRUE, 0);

  const uint vertColorU32 = vertColor.toRGBA();
  zeus::CVector2f uv1Min(0.f, 0.f);
  zeus::CVector2f uv1Max(1.f, 1.f);

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(topLeft.x, 0.5f, topLeft.y);
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.01f, 0.01f);
  GXTexCoord2f32(uv0Min.x(), uv0Min.y());
  GXTexCoord2f32(uv1Min.x(), uv1Min.y());

  GXPosition3f32(topLeft.x, 0.5f, bottomRight.y);
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.01f, 0.99f);
  GXTexCoord2f32(uv0Min.x(), uv0Max.y());
  GXTexCoord2f32(uv1Min.x(), uv1Max.y());

  GXPosition3f32(bottomRight.x, 0.5f, bottomRight.y);
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.99f, 0.99f);
  GXTexCoord2f32(uv0Max.x(), uv0Max.y());
  GXTexCoord2f32(uv1Max.x(), uv1Max.y());

  GXPosition3f32(bottomRight.x, 0.5f, topLeft.y);
  GXColor1u32(vertColorU32);
  GXTexCoord2f32(0.99f, 0.01f);
  GXTexCoord2f32(uv0Max.x(), uv0Min.y());
  GXTexCoord2f32(uv1Max.x(), uv1Min.y());

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProjection);
  CGraphics::SetViewPointMatrix(backupView);
}

void CCubeRenderer::ReallyDrawPhazonSuitEffect(const zeus::CColor& modColor, CTexture& maskTex) {
  maskTex.Load(CGraphics::kSpareBufferTexMapID, EClampMode::Repeat);

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

  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanAmbColor(CGX::EChannelId::Channel0, to_gx_color(zeus::skBlack));

  GXSetDstAlpha(GX_TRUE, 0);
  GXSetColorUpdate(GX_TRUE);

  const uint color = modColor.toRGBA();

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

  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
}

void* CCubeRenderer::GetRenderToTexBuffer(int idx) {
  return static_cast<u8*>(CGraphics::mpSpareBuffer) + (static_cast<uint>(idx * CGraphics::mSpareBufferSize) >> 4);
}

void CCubeRenderer::CopyTex(int div, bool half, void* dest, GXTexFmt fmt, bool clear) {
  const CViewport& vp = CGraphics::mViewport;
  uint width = vp.mWidth;
  uint height = vp.mHeight;
  const uint copyHeight = height / div;

  GXSetTexCopySrc(static_cast<u16>(vp.mLeft), static_cast<u16>(vp.mTop + vp.mHeight - copyHeight),
                  static_cast<u16>(vp.mWidth / div), static_cast<u16>(copyHeight));

  if (half) {
    width >>= 1;
    height >>= 1;
  }

  GXSetTexCopyDst(static_cast<u16>(width / div), static_cast<u16>(height / div), fmt, half);

  CGraphics::SetClearColor(zeus::skClear);

  GXSetColorUpdate(false);
  if (dest == nullptr) {
    dest = CGraphics::mpSpareBuffer;
  }
  GXCopyTex(dest, clear);
  GXSetColorUpdate(true);
  GXPixModeSync();
}

void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float blurRadius, float blurRadiusB) {
  const int width = CGraphics::mViewport.mWidth;
  const int height = CGraphics::mViewport.mHeight;

  CGraphics::SetOrtho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform());
  SetModelMatrix(zeus::CTransform());
  CGraphics::SetDepthWriteMode(false, ERglEnum::GEqual, false);

  CopyTex(1, true, GetRenderToTexBuffer(8), GX_TF_A8, true);
  GXSetDstAlpha(GX_TRUE, 0);

  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth / 2, LogicalFbHeight / 2, GX_TF_I8, GetRenderToTexBuffer(8),
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
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  GXSetColorUpdate(GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanMatColor(CGX::EChannelId::Channel0, to_gx_color(zeus::skWhite));
  CGX::SetChanAmbColor(CGX::EChannelId::Channel0, to_gx_color(zeus::skBlack));

  const uint white = zeus::skWhite.toRGBA();

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

  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth / 4, LogicalFbHeight / 4, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurOffsets[8][2] = {
      {-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}, {-1.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {0.f, -1.f},
  };

  const float blurScale = blurRadius * (2.f / static_cast<float>(width));
  const uint blurColorA = zeus::CColor(1.f, 1.f, 1.f, 0.3f).toRGBA();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    const zeus::CVector2f ofs(blurScale * blurOffsets[i][0], blurScale * blurOffsets[i][1]);

    GXPosition3f32(ofs.x(), 0.f, ofs.y());
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(ofs.x() + 0.25f, 0.f, ofs.y());
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(ofs.x(), 0.f, ofs.y() + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(ofs.x() + 0.25f, 0.f, ofs.y() + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }

  GXSetDstAlpha(GX_FALSE, 0);
  CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  const uint whiteColor = zeus::CColor(1.f, 1.f, 1.f, 1.f).toRGBA();

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

  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth / 4, LogicalFbHeight / 4, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurScaleB = blurRadiusB * (1.5f / static_cast<float>(width));
  const uint blurColorB = zeus::CColor(1.f, 1.f, 1.f, 0.35f).toRGBA();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    const zeus::CVector2f ofs(blurScaleB * blurOffsets[i][0], blurScaleB * blurOffsets[i][1]);

    GXPosition3f32(ofs.x(), 0.f, ofs.y());
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(0.25f + ofs.x(), 0.f, ofs.y());
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(ofs.x(), 0.f, 0.25f + ofs.y());
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(0.25f + ofs.x(), 0.f, 0.25f + ofs.y());
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }
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
        models->emplace_back(std::make_unique<CCubeModel>(
            const_cast<std::vector<CCubeSurface>*>(inst.GetSurfaces()), textures.get(),
            const_cast<u8*>(inst.GetMaterialPointer()), inst.GetVertexPointer(), inst.GetColorPointer(),
            inst.GetNormalPointer(), inst.GetTCPointer(), inst.GetPackedTCPointer(), inst.GetBoundingBox(),
            inst.GetFlags(), false, instIdx));
        ++instIdx;
      }
    }
    x1c_areaListItems.emplace_back(geometry, octTree, std::move(textures), std::move(models), areaIdx);
  }
}

void CCubeRenderer::EnablePVS(const CPVSVisSet& set, u32 areaIdx) {
  xc8_pvs.emplace(set);
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
      fmt::format("CCubeRenderer::DrawUnsortedGeometry areaIdx={} mask={} targetMask={}", areaIdx, mask, targetMask)
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
      fmt::format("CCubeRenderer::DrawSortedGeometry areaIdx={} mask={} targetMask={}", areaIdx, mask, targetMask)
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
      fmt::format("CCubeRenderer::DrawAreaGeometry areaIdx={} mask={} targetMask={}", areaIdx, mask, targetMask)
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
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format("CCubeRenderer::RenderBucketItems areaIdx={}", item->x18_areaIdx).c_str(),
                              zeus::skBlue);

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
    bool insideA = aabbA.pointInside(
        zeus::CVector3f(CGraphics::mViewMatrix.origin.x(), CGraphics::mViewMatrix.origin.y(), aabbA.min.z()));

    zeus::CAABox aabbB = b.x34_aabb.getTransformedAABox(b.x0_transform);
    bool insideB = aabbB.pointInside(
        zeus::CVector3f(CGraphics::mViewMatrix.origin.x(), CGraphics::mViewMatrix.origin.y(), aabbB.min.z()));

    if (insideA != insideB) {
      return insideA;
    }

    float dotA = aabbA.furthestPointAlongVector(CGraphics::mViewMatrix.basis[1]).dot(CGraphics::mViewMatrix.basis[1]);
    float dotB = aabbB.furthestPointAlongVector(CGraphics::mViewMatrix.basis[1]).dot(CGraphics::mViewMatrix.basis[1]);
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
    const bool invertTest = zOnly ? CGraphics::mViewMatrix.origin.z() >= plane.d()
                                  : plane.pointToPlaneDist(CGraphics::mViewMatrix.origin) >= 0.f;
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
  CGraphics::SetViewport(0, 0, CGraphics::mRenderModeObj.fbWidth, CGraphics::mRenderModeObj.xfbHeight);

  CGraphics::SetClearColor(zeus::skClear);
  CGraphics::SetCullMode(ERglCullMode::Front);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                          ERglLogicOp::Clear);
  CGraphics::SetPerspective(75.f, CGraphics::mViewport.mWidth / CGraphics::mViewport.mHeight, 1.f, 4096.f);
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

  GXSetPixelFmt(x318_27_currentRGBA6 ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GXSetAlphaUpdate(true);
  GXSetDstAlpha(true, 0.f);
  CGraphics::BeginScene();
}

void CCubeRenderer::EndScene() {
  x318_31_persistRGBA6 = !CGraphics::mIsBeginSceneClearFb;
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
  CGX::Begin(static_cast<GXPrimitive>(type), GX_VTXFMT0, nverts);
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

float CCubeRenderer::GetFPS() { return CGraphics::GetFPS(); }

CTexture* CCubeRenderer::GetRealReflection() {
  x2dc_reflectionAge = 0;
  if (x14c_reflectionTex) {
    return x14c_reflectionTex.get();
  }
  return &xe4_blackTex;
}

void CCubeRenderer::CacheReflection(TReflectionCallback cb, void* ctx, bool clearAfter) {
  if (x318_24_reflectionDirty) {
    x318_24_reflectionDirty = false;
    x2dc_reflectionAge = 0;

    if (!x14c_reflectionTex) {
      x14c_reflectionTex = std::make_unique<CTexture>(ETexelFormat::RGB565, 128, 128, 1, "Reflection texture");
    }

    const CViewport& vp = CGraphics::mViewport;
    const int oldLeft = vp.mLeft;
    const int oldTop = vp.mTop;
    const int oldWidth = vp.mWidth;
    const int oldHeight = vp.mHeight;

    const int captureTop = static_cast<int>(CGraphics::mRenderModeObj.efbHeight) - 250;
    CGraphics::SetViewport(0, captureTop, 256, 256);
    CGraphics::SetScissor(0, captureTop, 256, 256);

    void* spareBuffer = CGraphics::mpSpareBuffer;
    GXSetTexCopySrc(0, 0, 256, 256);
    GXSetTexCopyDst(128, 128, GX_TF_RGB565, true);
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(spareBuffer, true);

    cb(ctx, CCubeMaterial::GetViewingReflection());

    void* reflectionData = const_cast<u8*>(x14c_reflectionTex->GetConstBitMapData(0));
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(reflectionData, clearAfter);

    CGraphics::SetViewport(oldLeft, oldTop, oldWidth, oldHeight);
    CGraphics::SetScissor(oldLeft, oldTop, oldWidth, oldHeight);
  }
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
  Mtx mtx;
  xf.toCStyleMatrix(mtx);
  float f1 = -(1.f - t) * 6.f + 1.f;
  float f2 = t * -0.85f - 0.15f;
  const Mtx ptTex0 = {
      {1.0f, 1.0f, 0.0f, t},
      {0.0f, 0.0f, 1.0f, f1},
      {0.0f, 0.0f, 0.0f, 1.0f},
  };
  const Mtx ptTex1 = {
      {1.0f, 1.0f, 0.0f, f2},
      {0.0f, 0.0f, 1.0f, f1},
      {0.0f, 0.0f, 0.0f, 1.0f},
  };
  GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(ptTex0, GX_PTTEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(ptTex1, GX_PTTEXMTX1, GX_MTX3x4);
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
    x2ac_fogVolumes.emplace_back(CGraphics::mModelMatrix, color, aabb, model, sModel);
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
  GXCopyTex(CGraphics::mpSpareBuffer, true);
  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth, LogicalFbHeight, GX_TF_I4, nullptr, GX_TEXMAP7);

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
  const auto backupViewMatrix = CGraphics::mViewMatrix;
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
  GXCopyTex(CGraphics::mpSpareBuffer, false);
  x288_thermoPalette.Load();
  CGraphics::LoadDolphinSpareTexture(LogicalFbWidth, LogicalFbHeight, GX_TF_C4, GX_TLUT0, nullptr, GX_TEXMAP7);
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
  const auto backupViewMatrix = CGraphics::mViewMatrix;
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

void CCubeRenderer::DrawPhazonSuitIndirectEffect(const zeus::CColor& nonIndirectColor,
                                                 const TLockedToken<CTexture>& indirectTex,
                                                 const zeus::CColor& indirectColor, float blurRadius, float scale,
                                                 float offX, float offY) {
  if (x318_27_currentRGBA6 && x310_phazonSuitMaskCountdown != 0) {
    const zeus::CTransform backupView(CGraphics::mViewMatrix);
    const CGraphics::CProjectionState backupProjection = CGraphics::GetProjectionState();

    if (!x314_phazonSuitMask || x314_phazonSuitMask->GetWidth() != CGraphics::mViewport.mWidth >> 2 ||
        x314_phazonSuitMask->GetHeight() != CGraphics::mViewport.mHeight >> 2) {
      return;
    }

    DoPhazonSuitIndirectAlphaBlur(blurRadius, blurRadius);
    x314_phazonSuitMask->SetLocked(true);
    CopyTex(4, false, x314_phazonSuitMask->GetBitMapData(0), GX_TF_A8, true);

    CTexture* indTex = nullptr;
    if (indirectTex && ((indTex = const_cast<CTexture*>(indirectTex.GetObj())) != nullptr)) {
      ReallyDrawPhazonSuitIndirectEffect(zeus::CColor(1.f, 1.f, 1.f, 1.f), *x314_phazonSuitMask, *indTex, indirectColor,
                                         scale, offX, offY);
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

void CCubeRenderer::DrawXRayOutline(const zeus::CAABox& bounds) {
  for (const auto& item : x1c_areaListItems) {
    const CAreaRenderOctTree* octTree = item.x4_octTree;
    if (octTree == nullptr) {
      continue;
    }

    std::vector<uint> modelBits;
    octTree->FindOverlappingModels(modelBits, bounds);

    int wordModel = 0;
    for (uint i = 0; i < octTree->x14_bitmapWordCount; ++i, wordModel += 0x20) {
      const uint word = modelBits[i];
      if (word == 0) {
        continue;
      }

      for (int j = 0; j < 0x20; ++j) {
        if ((word & (1 << j)) == 0) {
          continue;
        }

        CCubeModel* modelInst = item.x10_models->at(wordModel + j).get();
        modelInst->SetArraysCurrent();

        for (CCubeSurface* surf = modelInst->GetFirstUnsortedSurface();
             surf != nullptr && surf->GetDisplayList() != nullptr; surf = surf->GetNextSurface()) {
          if (surf->GetBounds().intersects(bounds)) {
            const CCubeMaterial& mat = modelInst->GetMaterialByIndex(surf->GetMaterialIndex());
            CGX::SetVtxDescv_Compressed(mat.GetVertexDesc());
            CGX::CallDisplayList(surf->GetDisplayList(), surf->GetDisplayListSize());
          }
        }
      }
    }
  }
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

constexpr Mtx MvPostXf = {
    {0.5f, 0.0f, 0.0f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

void CCubeRenderer::DoThermalModelDraw(CCubeModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                                       TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::DoThermalModelDraw", zeus::skBlue);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, true, GX_PTTEXMTX0);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  x220_sphereRamp.Load(GX_TEXMAP0, EClampMode::Clamp);
  zeus::CTransform xf = CGraphics::mViewMatrix.quickInverse().multiplyIgnoreTranslation(CGraphics::mModelMatrix);
  xf.origin.zeroOut();
  Mtx mtx;
  xf.toCStyleMatrix(mtx);
  GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(MvPostXf, GX_PTTEXMTX0, GX_MTX3x4);
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

void CCubeRenderer::ReallyDrawSpaceWarp(const zeus::CVector3f& point, float strength) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCubeRenderer::ReallyDrawSpaceWarp", zeus::skBlue);
  const int vpWidth = CGraphics::mViewport.mWidth;
  const int vpHeight = CGraphics::mViewport.mHeight;
  const int vpTop = CGraphics::mViewport.mTop;
  const int vpLeft = CGraphics::mViewport.mLeft;

  const zeus::CVector3f& pointRef = zeus::CVector3f(point.x(), point.y(), point.z());
  const float projectedX =
      static_cast<float>(vpLeft) + static_cast<float>(vpWidth / 2) * pointRef.x() + static_cast<float>(vpWidth / 2);
  const float projectedY =
      static_cast<float>(vpTop) + static_cast<float>(vpHeight / 2) * -pointRef.y() + static_cast<float>(vpHeight / 2);

  zeus::CVector2i center(static_cast<int>(projectedX) & ~3, static_cast<int>(projectedY) & ~3);
  int size = (96.f / 448.f) * vpHeight;
  zeus::CVector2i v2right = center - zeus::CVector2i(size, size);
  zeus::CVector2i v2left = center + zeus::CVector2i(size, size);

  zeus::CVector2f uv1min(0.f, 0.f);
  zeus::CVector2f uv1max(1.f, 1.f);
  float* uv1minVals = reinterpret_cast<float*>(&uv1min);
  float* uv1maxVals = reinterpret_cast<float*>(&uv1max);

  const int alignedLeft = vpLeft & ~3;
  const int alignedTop = vpTop & ~3;
  const int alignedRight = (vpLeft + vpWidth + 3) & ~3;
  const int alignedBottom = (vpTop + vpHeight + 3) & ~3;

  if (v2right.x < alignedLeft) {
    uv1minVals[0] = (1.f / 192.f) * static_cast<float>(alignedLeft - v2right.x);
    v2right.x = alignedLeft;
  }

  if (v2right.y < alignedTop) {
    uv1minVals[1] = (1.f / 192.f) * static_cast<float>(alignedTop - v2right.y);
    v2right.y = alignedTop;
  }

  if (v2left.x > alignedRight) {
    uv1maxVals[0] = 1.f - (1.f / 192.f) * static_cast<float>(v2left.x - alignedRight);
    v2left.x = alignedRight;
  }

  if (v2left.y > alignedBottom) {
    uv1maxVals[1] = 1.f - (1.f / 192.f) * static_cast<float>(v2left.y - alignedBottom);
    v2left.y = alignedBottom;
  }

  const zeus::CVector2i v2sub = v2left - v2right;
  if (v2sub.x <= 0 || v2sub.y <= 0) {
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
  GXSetTexCopySrc(v2right.x, v2right.y, v2sub.x, v2sub.y);
  GXSetTexCopyDst(v2sub.x, v2sub.y, GX_TF_RGBA8, false);
  GXCopyTex(spareBuffer, false);
  GXPixModeSync();

  const int logicalCopyWidth = (v2sub.x * 448 + vpHeight / 2) / vpHeight;
  const int logicalCopyHeight = (v2sub.y * 448 + vpHeight / 2) / vpHeight;
  CGraphics::LoadDolphinSpareTexture(logicalCopyWidth, logicalCopyHeight, GX_TF_RGBA8, 0,
                                     CGraphics::kSpareBufferTexMapID);

  x150_reflectionTex.Load(GX_TEXMAP1, EClampMode::Clamp);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

  const float indScale = static_cast<float>(0.5 * strength);
  float indMtx[2][3] = {
      {indScale, 0.f, 0.f},
      {0.f, indScale, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, -1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, false,
                      false, GX_ITBA_OFF);
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
  zeus::CTransform backupViewMtx(CGraphics::mViewMatrix);

  CGraphics::SetOrtho(static_cast<float>(vpLeft), static_cast<float>(vpLeft + vpWidth), static_cast<float>(vpTop),
                      static_cast<float>(vpTop + vpHeight), -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform());
  CGraphics::SetModelMatrix(zeus::CTransform());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(static_cast<float>(v2right.x), 0.5f, static_cast<float>(v2right.y));
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(uv1minVals[0], uv1minVals[1]);

  GXPosition3f32(static_cast<float>(v2right.x), 0.5f, static_cast<float>(v2left.y));
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(uv1minVals[0], uv1maxVals[1]);

  GXPosition3f32(static_cast<float>(v2left.x), 0.5f, static_cast<float>(v2left.y));
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(uv1maxVals[0], uv1maxVals[1]);

  GXPosition3f32(static_cast<float>(v2left.x), 0.5f, static_cast<float>(v2right.y));
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(uv1maxVals[0], uv1minVals[1]);

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProj);
  CGraphics::SetViewPointMatrix(backupViewMtx);

  CGX::SetFog(fogType, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);
}

void CCubeRenderer::ReallyRenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const CModel* model,
                                          const CSkinnedModel* skinnedModel) {
  static const int skEdges[12][2] = {
      {0, 1}, {1, 3}, {3, 2}, {2, 0}, {4, 5}, {5, 7}, {7, 6}, {6, 4}, {0, 4}, {1, 5}, {3, 7}, {2, 6},
  };
  static const GXVtxDescList vtxDescrs[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  static int skMinX = 0;
  static int skMinY = 0;

  void* copyBufA = CGraphics::mpSpareBuffer;
  void* copyBufB = static_cast<u8*>(CGraphics::mpSpareBuffer) + 0x23000;

  const int vpLeft = CGraphics::mViewport.mLeft;
  const int vpTop = CGraphics::mViewport.mTop;
  const int vpWidth = CGraphics::mViewport.mWidth;
  const int vpHeight = CGraphics::mViewport.mHeight;

  int maxChunkW = 0x140;
  int chunkH = 0xe0;
  int chunkW = maxChunkW;
  int drawRight = vpWidth;
  int drawBottom = vpHeight;
  int drawLeft = 0;
  int drawTop = 0;
  bool recalcChunk = true;

  const zeus::CTransform modelXf(CGraphics::mModelMatrix);
  const zeus::CTransform viewXf(CGraphics::mViewMatrix);
  const zeus::CMatrix4f projMtx = CGraphics::GetPerspectiveProjectionMatrix();

  zeus::CVector2i minBounds(vpWidth, vpHeight);
  zeus::CVector2i maxBounds(0, 0);

  rstl::reserved_vector<zeus::CVector3f, 8> clipPts;
  rstl::reserved_vector<float, 8> clipWs;
  for (int i = 0; i < 8; ++i) {
    const zeus::CVector3f point = aabb.getPoint(i);
    const zeus::CVector3f worldPoint = modelXf * point;

    const zeus::CVector3f viewVec = worldPoint - viewXf.origin;
    const zeus::CVector3f rotated = viewXf.transposeRotate(viewVec);

    float w = 0.f;
    const auto pt = projMtx.multiplyOneOverW(rotated, w);
    clipPts.push_back(pt);
    clipWs.push_back(w);
  }

  bool allOutside = true;
  for (int i = 0; i < 20; ++i) {
    float ndcX;
    float ndcY;
    float ndcZ;
    bool validPoint = false;

    if (i < 8) {
      const float invW = 1.f / clipWs[i];
      ndcX = invW * clipPts[i].x();
      ndcY = invW * clipPts[i].y();
      ndcZ = invW * clipPts[i].z();
      validPoint = true;
    } else {
      const int idxA = skEdges[i - 8][0];
      const int idxB = skEdges[i - 8][1];

      const float wA = clipWs[idxA];
      const float wB = clipWs[idxB];
      const zeus::CVector3f& pA = clipPts[idxA];
      const zeus::CVector3f& pB = clipPts[idxB];

      if ((1.f < pA.z() / wA) != (1.f < pB.z() / wB)) {
        const float t = -(wA - 1.f) / (wB - wA);
        if (0.f < t && t < 1.f) {
          const float invW = 1.f / (wA + t * (wB - wA));
          ndcX = invW * (pA.x() + t * (pB.x() - pA.x()));
          ndcY = invW * (pA.y() + t * (pB.y() - pA.y()));
          ndcZ = invW * (pA.z() + t * (pB.z() - pA.z()));
          validPoint = true;
        }
      }
    }

    if (!validPoint || ndcZ > 1.001f) {
      continue;
    }

    const int scrX = static_cast<int>(static_cast<float>(vpWidth) * ndcX * 0.5f + static_cast<float>(vpWidth / 2));
    const int scrY = static_cast<int>(static_cast<float>(vpHeight) * -ndcY * 0.5f + static_cast<float>(vpHeight / 2));

    const int p0 = std::max(skMinX, scrX) & ~3;
    const int p1 = std::max(skMinY, scrY) & ~3;
    const int p2 = std::min(vpWidth - 4, scrX + 3) & ~3;
    const int p3 = std::min(vpHeight - 4, scrY + 3) & ~3;

    if (p0 < minBounds.x) {
      minBounds.x = p0;
    }
    if (p1 < minBounds.y) {
      minBounds.y = p1;
    }
    if (p2 > maxBounds.x) {
      maxBounds.x = p2;
    }
    if (p3 > maxBounds.y) {
      maxBounds.y = p3;
    }

    allOutside = false;
  }

  if (!allOutside) {
    maxChunkW = std::min(maxChunkW, maxBounds.x - minBounds.x);
    chunkH = std::min(chunkH, maxBounds.y - minBounds.y);

    drawRight = std::min(vpWidth, maxBounds.x);
    drawBottom = std::min(vpHeight, maxBounds.y);
    drawLeft = minBounds.x;
    drawTop = minBounds.y;
  }

  if (maxChunkW <= 0 || chunkH <= 0) {
    return;
  }

  if (((drawTop + vpTop) & 1) != 0) {
    --drawTop;
  }
  if (((drawLeft + vpLeft) & 1) != 0) {
    --drawLeft;
  }

  bool oldVideoFilter = CGraphics::mUseVideoFilter;
  CGraphics::SetUseVideoFilter(false);

  float fogTexMtx[2][4] = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
  };
  fogTexMtx[0][3] = 0.5f / static_cast<float>(x1b8_fogVolumeRamp.GetWidth());
  fogTexMtx[1][3] = 0.5f / static_cast<float>(x1b8_fogVolumeRamp.GetHeight());
  GXLoadTexMtxImm(fogTexMtx, GX_TEXMTX0, GX_MTX2x4);

  const zeus::CVector3f maxPt = modelXf * aabb.max;
  const zeus::CVector3f minPt = modelXf * aabb.min;
  const zeus::CAABox modelAabb(zeus::CVector3f(minPt.x() - 1.f, minPt.y() - 1.f, minPt.z() - 1.f),
                               zeus::CVector3f(maxPt.x() + 1.f, maxPt.y() + 1.f, maxPt.z() + 1.f));

  bool doDoublePass = false;
  const zeus::CVector3f camPos = CGraphics::mViewMatrix.origin;
  if (modelAabb.pointInside(camPos) && (model != 0 || skinnedModel != 0)) {
    doDoublePass = true;
  }
  if (doDoublePass) {
    x318_26_requestRGBA6 = true;
    if (!x318_27_currentRGBA6) {
      doDoublePass = false;
    }
  }

  CGX::SetIndTexMtxSTPointFive(GX_ITM_0, 1);

  chunkW = maxChunkW;
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

      float uv0MaxX = 0.f;
      float uv0MaxY = 0.f;
      float uv0MinX = 0.f;
      float uv0MinY = 0.f;
      if (recalcChunk) {
        uv0MaxX = static_cast<float>(chunkW - 1) / static_cast<float>(chunkW);
        uv0MaxY = static_cast<float>(chunkH - 1) / static_cast<float>(chunkH);

        GXSetTexCopyDst(static_cast<u16>(chunkW), static_cast<u16>(chunkH), GX_CTF_A8, GX_FALSE);

        uv0MinX = 0.5f / static_cast<float>(chunkW);
        uv0MinY = 0.5f / static_cast<float>(chunkH);
      }

      const int copyLeft = x + vpLeft;

      GXSetTexCopySrc(static_cast<u16>(copyLeft), static_cast<u16>(copyTop), static_cast<u16>(chunkW),
                      static_cast<u16>(chunkH));
      GXSetScissorRender(copyLeft, copyTop, chunkW, chunkH);

      CGX::SetZMode(true, GX_LEQUAL, true);
      CGX::SetNumTevStages(1);
      CGX::SetNumTexGens(1);
      CGX::SetNumChans(0);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
      x1b8_fogVolumeRamp.Load(GX_TEXMAP2, EClampMode::Clamp);
      GXSetCullMode(GX_CULL_BACK);
      GXSetDstAlpha(GX_TRUE, 0xff);
      RenderFogVolumeModel(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(false, GX_ALWAYS, false);
        RenderFogVolumeModel(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      GXSetDstAlpha(GX_TRUE, 0);
      GXCopyTex(copyBufA, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufA, GX_TEXMAP0);

      GXSetCullMode(GX_CULL_FRONT);
      RenderFogVolumeModel(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(true, GX_GREATER, false);
        RenderFogVolumeModel(aabb, model, modelXf, CGraphics::mViewMatrix, skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      GXCopyTex(copyBufB, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufB, GX_TEXMAP1);

      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
      CGX::SetTevKColor(GX_KCOLOR0, to_gx_color(color));
      GXInvalidateTexAll();

      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
      CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
      CGX::SetNumTexGens(2);
      CGX::SetNumChans(0);

      const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
      CGX::SetVtxDescv(vtxDescrs);
      const zeus::CTransform oldView(CGraphics::mViewMatrix);

      CGraphics::SetOrtho(0.f, static_cast<float>(vpWidth), 0.f, static_cast<float>(vpHeight), -4096.f, 4096.f);
      CGraphics::SetViewPointMatrix(zeus::CTransform());
      CGraphics::SetModelMatrix(zeus::CTransform());

      CGX::SetZMode(false, GX_ALWAYS, false);
      GXSetCullMode(GX_CULL_NONE);
      GXSetAlphaUpdate(GX_FALSE);

      const float uv1MaxY = uv0MinY + uv0MaxY;
      const float uv1MaxX = uv0MinX + uv0MaxX;

      const int rightAbs = copyLeft + chunkW;
      const int bottom = y + chunkH;
      const int passCount = doDoublePass ? 2 : 1;
      for (int pass = 0; pass < passCount; ++pass) {
        if (pass == 0) {
          CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF,
                              false, false, GX_ITBA_OFF);
          CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF,
                              false, false, GX_ITBA_OFF);
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
        } else {
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

        GXPosition3f32(static_cast<float>(copyLeft), 0.5f, static_cast<float>(y));
        GXTexCoord2f32(uv0MinX, uv0MinY);

        GXPosition3f32(static_cast<float>(copyLeft), 0.5f, static_cast<float>(bottom));
        GXTexCoord2f32(uv0MinX, uv1MaxY);

        GXPosition3f32(static_cast<float>(rightAbs), 0.5f, static_cast<float>(bottom));
        GXTexCoord2f32(uv1MaxX, uv1MaxY);

        GXPosition3f32(static_cast<float>(rightAbs), 0.5f, static_cast<float>(y));
        GXTexCoord2f32(uv1MaxX, uv0MinY);

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

  GXSetScissorRender(vpLeft, vpTop, vpWidth, vpHeight);
  CGraphics::SetUseVideoFilter(oldVideoFilter);
}

void CCubeRenderer::RenderFogVolumeModel(const zeus::CAABox& aabb, const CModel* model, const zeus::CTransform& modelXf,
                                         const zeus::CTransform& viewXf, const CSkinnedModel* skinnedModel) {
  if (model == nullptr && skinnedModel == nullptr) {
    const zeus::CAABox transformedAabb = aabb.getTransformedAABox(modelXf);
    zeus::CAABox worldAabb = transformedAabb;

    static const GXVtxDescList vtxDescrs2[2] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    CGX::SetVtxDescv(vtxDescrs2);

    const zeus::CUnitVector3f viewPlaneFwd(viewXf.frontVector());
    const zeus::CVector3f min = worldAabb.min;
    const zeus::CVector3f max = -worldAabb.max;

    zeus::CPlane fogPlanes[7] = {
        zeus::CPlane(zeus::skRight, min.x()),
        zeus::CPlane(zeus::skLeft, max.x()),
        zeus::CPlane(zeus::skForward, min.y()),
        zeus::CPlane(zeus::skBack, max.y()),
        zeus::CPlane(zeus::skUp, min.z()),
        zeus::CPlane(zeus::skDown, max.z()),
        zeus::CPlane(viewPlaneFwd, viewXf.origin.dot(viewPlaneFwd) + 0.2f + 0.1f),
    };
    CGraphics::SetModelMatrix({});

    zeus::CVector3f extents = worldAabb.extents();
    float maxExtent = std::max(extents.x(), std::max(extents.y(), extents.z()));

    const float sliceExtent = maxExtent * 2.f;
    for (int i = 0; i < 7; ++i) {
      DrawFogSlices(fogPlanes, 7, i, worldAabb.center(), sliceExtent);
    }
  } else if (skinnedModel != nullptr) {
    CModel* skModel = const_cast<CModel*>(skinnedModel->GetModel().GetObj());
    skModel->Touch(0);
    if (CCubeModel* modelInst = &skModel->GetInstance()) {
      skModel->UpdateLastFrame();
      modelInst->DrawFlat(skModel->GetPositions(), skModel->GetNormals(), ESurfaceSelection::All);
    }
  } else {
    CModel* skModel = const_cast<CModel*>(model);
    skModel->Touch(0);
    if (CCubeModel* modelInst = &skModel->GetInstance()) {
      skModel->UpdateLastFrame();
      modelInst->DrawFlat({}, {}, ESurfaceSelection::All);
    }
  }
}

void CCubeRenderer::DrawFogSlices(const zeus::CPlane* planes, int planeCount, int planeIdx,
                                  const zeus::CVector3f& point, float extent) {
  static const int skEdges[3][2] = {
      {1, 2},
      {0, 2},
      {0, 1},
  };

  const zeus::CPlane& plane = planes[planeIdx];

  int axis = 0;
  if (fabsf(plane.normal().x()) < fabsf(plane.normal().y())) {
    axis = 1;
  }

  const float axisVal = axis == 0 ? plane.normal().x() : plane.normal().y();
  if (fabsf(axisVal) < fabsf(plane.normal().z())) {
    axis = 2;
  }

  const float dist = plane.normal().dot(point) - plane.d();
  const zeus::CVector3f projectedPoint(point.x() - dist * plane.normal().x(), point.y() - dist * plane.normal().y(),
                                       point.z() - dist * plane.normal().z());

  float axisSign = (axis == 0   ? plane.normal().x()
                    : axis == 1 ? plane.normal().y()
                                : plane.normal().z()) >= 0.f
                       ? -1.f
                       : 1.f;
  if (axis == 1) {
    axisSign = -axisSign;
  }

  double offsetA[3] = {0.0, 0.0, 0.0};
  double offsetB[3] = {0.0, 0.0, 0.0};
  offsetA[skEdges[axis][0]] = extent;
  offsetB[skEdges[axis][1]] = static_cast<double>(extent * axisSign);

  double base[3] = {projectedPoint.x(), projectedPoint.y(), projectedPoint.z()};
  double cornerDbl[4][3];
  int cornerCount = 0;

  cornerDbl[cornerCount][0] = base[0] - offsetA[0] - offsetB[0];
  cornerDbl[cornerCount][1] = base[1] - offsetA[1] - offsetB[1];
  cornerDbl[cornerCount][2] = base[2] - offsetA[2] - offsetB[2];
  ++cornerCount;

  cornerDbl[cornerCount][0] = base[0] + offsetA[0] - offsetB[0];
  cornerDbl[cornerCount][1] = base[1] + offsetA[1] - offsetB[1];
  cornerDbl[cornerCount][2] = base[2] + offsetA[2] - offsetB[2];
  ++cornerCount;

  cornerDbl[cornerCount][0] = base[0] + offsetA[0] + offsetB[0];
  cornerDbl[cornerCount][1] = base[1] + offsetA[1] + offsetB[1];
  cornerDbl[cornerCount][2] = base[2] + offsetA[2] + offsetB[2];
  ++cornerCount;

  cornerDbl[cornerCount][0] = base[0] - offsetA[0] + offsetB[0];
  cornerDbl[cornerCount][1] = base[1] - offsetA[1] + offsetB[1];
  cornerDbl[cornerCount][2] = base[2] - offsetA[2] + offsetB[2];
  ++cornerCount;

  zeus::CVector3f corners[4];
  int clipCornerCount = 0;
  for (int i = 0; i < 4; ++i) {
    const double planeDot = cornerDbl[i][0] * plane.normal().x() + cornerDbl[i][1] * plane.normal().y() +
                            cornerDbl[i][2] * plane.normal().z() - plane.d();

    corners[clipCornerCount++] = zeus::CVector3f(static_cast<float>(cornerDbl[i][0] - planeDot * plane.normal().x()),
                                                 static_cast<float>(cornerDbl[i][1] - planeDot * plane.normal().y()),
                                                 static_cast<float>(cornerDbl[i][2] - planeDot * plane.normal().z()));
  }

  DrawFogFans(planes, planeCount, corners, cornerCount, planeIdx, 0);
}

void CCubeRenderer::DrawFogFans(const zeus::CPlane* planes, int planeCount, const zeus::CVector3f* verts, int vertCount,
                                int startPlane, int curPlane) {
  if (curPlane == startPlane) {
    DrawFogFans(planes, planeCount, verts, vertCount, startPlane, curPlane + 1);
    return;
  }

  if (curPlane == planeCount) {
    DrawFogFan(verts, vertCount);
    return;
  }

  int clippedVertCount = 0;
  u8 clippedFlags[20];
  int clippedFlagCount = 0;

  const zeus::CPlane& plane = planes[curPlane];
  for (int i = 0; i < vertCount; ++i) {
    clippedFlags[clippedFlagCount++] = plane.normal().dot(verts[i]) >= plane.d() ? 0 : 1;
  }

  zeus::CVector3f clippedVerts[20];
  for (int i = 0; i < vertCount; ++i) {
    const uint next = static_cast<uint>(i + 1) & static_cast<uint>((i - (vertCount - 1) | (vertCount - 1) - i) >> 31);
    const uint clippedMask = static_cast<uint>(clippedFlags[i]) | (static_cast<uint>(clippedFlags[next]) << 1);

    if ((clippedFlags[i] & 1) == 0) {
      clippedVerts[clippedVertCount++] = verts[i];
    }

    if (clippedMask == 1 || clippedMask == 2) {
      const float t = plane.clipLineSegment(verts[i], verts[next]);
      if (0.f < t && t < 1.f) {
        const float invT = 1.f - t;
        clippedVerts[clippedVertCount++] =
            zeus::CVector3f(verts[i].x() * invT + verts[next].x() * t, verts[i].y() * invT + verts[next].y() * t,
                            verts[i].z() * invT + verts[next].z() * t);
      }
    }
  }

  if (clippedVertCount > 2) {
    DrawFogFans(planes, planeCount, clippedVerts, clippedVertCount, startPlane, curPlane + 1);
  }
}

void CCubeRenderer::DrawFogFan(const zeus::CVector3f* verts, int vertCount) {
  if (vertCount < 3) {
    return;
  }

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<ushort>(vertCount));
  for (int i = 0; i < vertCount; ++i) {
    GXPosition3f32(verts[i].x(), verts[i].y(), verts[i].z());
  }
  CGX::End();
}
} // namespace metaforce
