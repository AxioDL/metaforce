#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

#include "Runtime/CStopwatch.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/GameGlobalObjects.hpp"

#include "hecl/Backend.hpp"

#include "zeus/CAABox.hpp"

namespace urde {

void CModelShaders::FragmentUniform::ActivateLights(const std::vector<CLight>& lts) {
  ambient = zeus::skClear;
  size_t curLight = 0;

  for (const CLight& light : lts) {
    switch (light.GetType()) {
    case ELightType::LocalAmbient:
      ambient += light.GetColor();
      break;
    case ELightType::Point:
    case ELightType::Spot:
    case ELightType::Custom:
    case ELightType::Directional: {
      if (curLight >= lights.size()) {
        continue;
      }
      CModelShaders::Light& lightOut = lights[curLight++];
      lightOut.pos = CGraphics::g_CameraMatrix * light.GetPosition();
      lightOut.dir = (CGraphics::g_CameraMatrix.basis * light.GetDirection()).normalized();
      lightOut.color = light.GetColor();
      lightOut.linAtt[0] = light.GetAttenuationConstant();
      lightOut.linAtt[1] = light.GetAttenuationLinear();
      lightOut.linAtt[2] = light.GetAttenuationQuadratic();
      lightOut.angAtt[0] = light.GetAngleAttenuationConstant();
      lightOut.angAtt[1] = light.GetAngleAttenuationLinear();
      lightOut.angAtt[2] = light.GetAngleAttenuationQuadratic();

      if (light.GetType() == ELightType::Directional)
        lightOut.pos = (-lightOut.dir) * 1048576.f;
      break;
    }
    }
  }

  for (; curLight < lights.size(); ++curLight) {
    CModelShaders::Light& lightOut = lights[curLight];
    lightOut.pos = zeus::skZero3f;
    lightOut.dir = zeus::skDown;
    lightOut.color = zeus::skClear;
    lightOut.linAtt[0] = 1.f;
    lightOut.linAtt[1] = 0.f;
    lightOut.linAtt[2] = 0.f;
    lightOut.angAtt[0] = 1.f;
    lightOut.angAtt[1] = 0.f;
    lightOut.angAtt[2] = 0.f;
  }
}

using TexCoordSource = hecl::Backend::TexCoordSource;

constexpr std::array<hecl::Backend::TextureInfo, 1> ThermalTextures{{
    {TexCoordSource::Normal, 7, true},
}};

constexpr std::array<hecl::Backend::TextureInfo, 3> BallFadeTextures{{
    {TexCoordSource::Position, 0, false}, // ID tex
    {TexCoordSource::Position, 0, false}, // Sphere ramp
    {TexCoordSource::Position, 1, false}, // TXTR_BallFade
}};

constexpr std::array<hecl::Backend::TextureInfo, 1> WorldShadowTextures{{
    {TexCoordSource::Position, 7, false}, // Shadow tex
}};

constexpr std::array<hecl::Backend::TextureInfo, 2> DisintegrateTextures{{
    {TexCoordSource::Position, 0, false}, // Ashy tex
    {TexCoordSource::Position, 1, false}, // Ashy tex
}};

static std::array<hecl::Backend::ExtensionSlot, size_t(EExtendedShader::MAX)> g_ExtensionSlots{{
    /* Default solid shading */
    {},
    /* Normal lit shading */
    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
    /* Thermal Visor shading */
    {1, ThermalTextures.data(), hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::One,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, false, true},
    /* Forced alpha shading */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
    /* Forced additive shading */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
     hecl::Backend::CullMode::Backface, false, false, true},
    /* Solid color */
    {0, nullptr, hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
     hecl::Backend::CullMode::Backface, false, false, false},
    /* Solid color additive */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
     hecl::Backend::CullMode::Backface, true, false, true},
    /* Alpha-only Solid color frontface cull, LEqual */
    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
     hecl::Backend::CullMode::Frontface, false, true, false},
    /* Alpha-only Solid color frontface cull, Always, No Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::None,
     hecl::Backend::CullMode::Frontface, true, true, false},
    /* Alpha-only Solid color backface cull, LEqual */
    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
     hecl::Backend::CullMode::Backface, false, true, false},
    /* Alpha-only Solid color backface cull, Greater, No Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
     hecl::Backend::CullMode::Backface, true, true, false},
    /* MorphBall shadow shading */
    {3, BallFadeTextures.data(), hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::Equal, hecl::Backend::CullMode::Backface, false, false, true, false, true},
    /* World shadow shading (modified lighting) */
    {1, WorldShadowTextures.data(), hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
    /* Forced alpha shading without culling */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::None, false, false, true},
    /* Forced additive shading without culling */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
     hecl::Backend::CullMode::None, false, false, true},
    /* Forced alpha shading without Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Original, true, false, true},
    /* Forced additive shading without Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
     hecl::Backend::CullMode::Original, true, false, true},
    /* Forced alpha shading without culling or Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::None, true, false, true},
    /* Forced additive shading without culling or Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
     hecl::Backend::CullMode::None, true, false, true},
    /* Depth GEqual no Z-write */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::GEqual, hecl::Backend::CullMode::Backface, true, false, true},
    /* Disintegration */
    {2, DisintegrateTextures.data(), hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
     hecl::Backend::ZTest::LEqual, hecl::Backend::CullMode::Original, false, false, true, false, false, true},
    /* Forced additive shading without culling or Z-write and greater depth test */
    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
     hecl::Backend::CullMode::None, true, false, true},
    /* Thermal cold shading */
    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Original, false, false, true, false, false, false, true},
    /* Normal lit shading with alpha */
    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface},
    /* Normal lit shading with alpha without Z-write or depth test */
    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::None, hecl::Backend::CullMode::Backface, true},
    /* Normal lit shading with cube reflection */
    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
    /* Normal lit shading with cube reflection and world shadow */
    {1, WorldShadowTextures.data(), hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
}};

// TODO: put somewhere common
template <typename BitType>
class Flags {
public:
  using MaskType = typename std::underlying_type<BitType>::type;

  // constructors
  constexpr Flags() noexcept : m_mask(0) {}

  constexpr Flags(BitType bit) noexcept : m_mask(static_cast<MaskType>(bit)) {}

  constexpr Flags(Flags<BitType> const& rhs) noexcept : m_mask(rhs.m_mask) {}

  constexpr explicit Flags(MaskType flags) noexcept : m_mask(flags) {}

  // relational operators
  auto operator<=>(Flags<BitType> const&) const = default;

  // logical operator
  constexpr bool operator!() const noexcept { return !m_mask; }

  // bitwise operators
  constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(m_mask & rhs.m_mask);
  }

  constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(m_mask | rhs.m_mask);
  }

  constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(m_mask ^ rhs.m_mask);
  }

  // assignment operators
  constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept {
    m_mask = rhs.m_mask;
    return *this;
  }

  constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept {
    m_mask |= rhs.m_mask;
    return *this;
  }

  constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept {
    m_mask &= rhs.m_mask;
    return *this;
  }

  constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept {
    m_mask ^= rhs.m_mask;
    return *this;
  }

  // cast operators
  explicit constexpr operator bool() const noexcept { return !!m_mask; }

  explicit constexpr operator MaskType() const noexcept { return m_mask; }

private:
  MaskType m_mask;
};

enum class CCubeMaterialFlagBits : u32 {
  fKonstValues = 0x8,
  fDepthSorting = 0x10,
  fAlphaTest = 0x20,
  fSamusReflection = 0x40,
  fDepthWrite = 0x80,
  fSamusReflectionSurfaceEye = 0x100,
  fShadowOccluderMesh = 0x200,
  fSamusReflectionIndirectTexture = 0x400,
  fLightmap = 0x800,
  fLightmapUvArray = 0x2000,
  fTextureSlotMask = 0xffff0000
};
using CCubeMaterialFlags = Flags<CCubeMaterialFlagBits>;

enum class CCubeMaterialVatAttribute : u32 {
  Position = 0,
  Normal = 2,
  Color0 = 4,
  Color1 = 8,
  Tex0 = 10,
  Tex1 = 12,
  Tex2 = 14,
  Tex3 = 16,
  Tex4 = 18,
  Tex5 = 20,
  Tex6 = 22,
};
enum class CCubeMaterialVatAttributeType : u32 { None = 0, Direct = 1, Index8 = 2, Index16 = 3 };
class CCubeMaterialVatFlags {
  u32 m_flags;

public:
  constexpr CCubeMaterialVatFlags() noexcept : m_flags(0) {}
  CCubeMaterialVatAttributeType GetAttributeType(CCubeMaterialVatAttribute attribute) const noexcept {
    return CCubeMaterialVatAttributeType((m_flags >> u32(attribute)) & 0x3);
  }
  void SetAttributeType(CCubeMaterialVatAttribute attribute, CCubeMaterialVatAttributeType type) noexcept {
    m_flags &= ~(u32(0x3) << u32(attribute));
    m_flags |= u32(type) << u32(attribute);
  }
};

struct CCubeSurface {
  float x0_center[3];
  u32 xc_matIdx;
  u16 x10_fixedPointRange;
  u16 x12_dlSize;
  struct CCubeModel* x14_parentModel;
  CCubeSurface* x18_nextSurface;
  u32 x1c_extraSize;
  float x20_normal[3];
};

struct CMetroidModelInstance {
  u32 x0_visorFlags;
  zeus::CTransform x4_worldXf;
  zeus::CAABox x34_worldAABB;
  void* x4c_firstGeomData;
  std::vector<CCubeSurface*> x50_surfacePtrs;
  void* x60_positions;
  void* x64_normals;
  void* x68_vtxColors;
  void* x6c_floatUVs;
  void* x70_shortUVs;
  // CMetroidModelInstance(void* modelHeader, void* firstGeomPtr, void* positions, void* normals, void* vtxColors,
  //                      void* floatUVs, void* shortUVs, const std::vector<CCubeSurface*>& surfacePtrs);
};

struct STexReference {
  u32 x0_;
  u32 x4_;
  CTexture* x8_tex;
};

struct CCubeModel {
  static bool sUsingPackedLightmaps;
  const std::vector<CCubeSurface*>& x0_surfacePtrs;
  void* x4_firstGeomData;
  void* x8_positions;
  void* xc_normals;
  void* x10_vtxColors;
  void* x14_floatUVs;
  void* x18_shortUVs;
  const std::vector<STexReference>& x1c_texs;
  zeus::CAABox x20_worldAABB;
  CCubeSurface* x38_firstUnsortedSurf = nullptr;
  CCubeSurface* x3c_firstSortedSurf = nullptr;
  bool x40_24_someFlag;
  bool x40_25 = false;
  u8 x41_visorFlags;
  u32 x44_idx = 0;
  // CCubeModel(const std::vector<CCubeSurface*>& surfacePtrs, const std::vector<STexReference>& texs,
  //           void* firstGeomData, void* positions, void* normals, void* vtxColors,
  //           void* floatUVs, void* shortUVs, const zeus::CAABox& worldAABB, u8 visorFlags,
  //           bool someFlag, u32 surfaceIdx);

  void SetUsingPackedLightmaps() const {}
};

static const u8* sLastMaterialCached = nullptr;
static bool sRenderModelBlack = false;
static u32 sLastMaterialUnique = 0;
static bool kShadowMapsEnabled = false;

enum class CModelFlagsFlagBits : u16 {
  fDepthTest = 0x1,
  fDepthWrite = 0x2,
  fDrawWithoutTexLock = 0x4,
  kDepthGreater = 0x8,
  fDepthNonInclusive = 0x10,
};
using CModelFlagsFlags = Flags<CModelFlagsFlagBits>;

using namespace hsh::pipeline;

constexpr hsh::sampler Samp;
constexpr hsh::sampler ClampSamp(hsh::Linear, hsh::Linear, hsh::Linear, hsh::ClampToBorder, hsh::ClampToBorder,
                                 hsh::ClampToBorder, 0.f, hsh::Never, hsh::OpaqueWhite);
constexpr hsh::sampler ClampEdgeSamp(hsh::Linear, hsh::Linear, hsh::Linear, hsh::ClampToEdge, hsh::ClampToEdge,
                                     hsh::ClampToEdge);
constexpr hsh::sampler ReflectSamp(hsh::Linear, hsh::Linear, hsh::Linear, hsh::ClampToBorder, hsh::ClampToBorder,
                                   hsh::ClampToBorder);

template <bool CubeReflection>
struct DynReflectionTex {
  using type = hsh::texture2d;
};
template <>
struct DynReflectionTex<true> {
  using type = hsh::texturecube;
};
template <bool CubeReflection>
using DynReflectionTexType = typename DynReflectionTex<CubeReflection>::type;

using BlendMaterial = hecl::blender::Material;
using MaterialBlendMode = BlendMaterial::BlendMode;
using MaterialTexCoordSource = BlendMaterial::TexCoordSource;

struct NoColorValue {
  static constexpr bool Constant = false;
};

template <float R, float G, float B, float A>
struct ColorValue {
  static constexpr bool Constant = true;
  static constexpr hsh::float3 RGB{R, G, B};
  static constexpr float A_ = A;
};

template <MaterialTexCoordSource Source, bool Normalize, int MtxIdx, bool SampleAlpha, class ConstantColor = NoColorValue>
struct PassTraits {
  static constexpr MaterialTexCoordSource Source_ = Source;
  static constexpr bool Normalize_ = Normalize;
  static constexpr int MtxIdx_ = MtxIdx;
  static constexpr bool SampleAlpha_ = SampleAlpha;
  using ConstantColor_ = ConstantColor;
};

template <MaterialBlendMode Mode>
constexpr hsh::BlendFactor MaterialBlendModeSrcFactor = hsh::One;
template <>
constexpr hsh::BlendFactor MaterialBlendModeSrcFactor<MaterialBlendMode::Alpha> = hsh::SrcAlpha;
template <>
constexpr hsh::BlendFactor MaterialBlendModeSrcFactor<MaterialBlendMode::Additive> = hsh::SrcAlpha;

template <MaterialBlendMode Mode>
constexpr hsh::BlendFactor MaterialBlendModeDstFactor = hsh::Zero;
template <>
constexpr hsh::BlendFactor MaterialBlendModeDstFactor<MaterialBlendMode::Alpha> = hsh::InvSrcAlpha;
template <>
constexpr hsh::BlendFactor MaterialBlendModeDstFactor<MaterialBlendMode::Additive> = hsh::One;

template <bool ColorUpdate, bool AlphaUpdate>
constexpr hsh::ColorComponentFlags
    ColorUpdateFlags = hsh::ColorComponentFlags((ColorUpdate ? hsh::CC_Red | hsh::CC_Green | hsh::CC_Blue : 0) |
                                                (AlphaUpdate ? hsh::CC_Alpha : 0));

template <MaterialBlendMode MaterialBlendMode, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachmentBase
: color_attachment<MaterialBlendModeSrcFactor<MaterialBlendMode>, MaterialBlendModeDstFactor<MaterialBlendMode>,
                   hsh::Add, DstAlpha ? hsh::ConstAlpha : hsh::Zero, hsh::Zero, hsh::Add,
                   ColorUpdateFlags<ColorUpdate, AlphaUpdate>> {};

template <hsh::BlendFactor Src, hsh::BlendFactor Dst, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachmentOverride
: color_attachment<Src, Dst, hsh::Add, DstAlpha ? hsh::ConstAlpha : hsh::Zero, hsh::Zero, hsh::Add,
                   ColorUpdateFlags<ColorUpdate, AlphaUpdate>> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment0To4
: CModelShadersColorAttachmentBase<MaterialBlendMode, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment0To4<MaterialBlendMode, true, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachmentOverride<hsh::One, hsh::Zero, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment5To6
: CModelShadersColorAttachmentBase<MaterialBlendMode, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment5To6<MaterialBlendMode::Opaque, false, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachmentOverride<hsh::SrcAlpha, hsh::InvSrcAlpha, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment5To6<MaterialBlendMode, true, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachmentOverride<hsh::SrcAlpha, hsh::InvSrcAlpha, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment7To8
: CModelShadersColorAttachmentBase<MaterialBlendMode, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment7To8<MaterialBlendMode::Opaque, false, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachmentOverride<hsh::SrcAlpha, hsh::One, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment7To8<MaterialBlendMode, true, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachmentOverride<hsh::SrcAlpha, hsh::One, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, u8 GameBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate,
          bool DstAlpha>
struct CModelShadersColorAttachment : color_attachment<> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 0, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment0To4<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 1, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment0To4<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 2, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment0To4<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 3, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment0To4<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 4, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment0To4<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 5, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment5To6<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 6, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment5To6<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 7, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment7To8<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, bool AlphaTest, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersColorAttachment<MaterialBlendMode, 8, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha>
: CModelShadersColorAttachment7To8<MaterialBlendMode, AlphaTest, ColorUpdate, AlphaUpdate, DstAlpha> {};

template <MaterialBlendMode MaterialBlendMode, u32 MaterialFlags, u8 GameBlendMode, u16 ModelFlags, bool ColorUpdate,
          bool AlphaUpdate, bool DstAlpha>
struct CModelShadersPipelineConfig
: pipeline<
      CModelShadersColorAttachment<MaterialBlendMode, GameBlendMode,
                                   MaterialFlags & u16(CCubeMaterialFlagBits::fAlphaTest), ColorUpdate, AlphaUpdate,
                                   DstAlpha>,
      depth_compare<ModelFlags & u16(CModelFlagsFlagBits::fDepthTest)
                        ? (ModelFlags& u16(CModelFlagsFlagBits::kDepthGreater)
                               ? (ModelFlags& u16(CModelFlagsFlagBits::fDepthNonInclusive) ? hsh::Greater : hsh::GEqual)
                               : (ModelFlags& u16(CModelFlagsFlagBits::fDepthNonInclusive) ? hsh::Less : hsh::LEqual))
                        : hsh::Always>,
      depth_write<ModelFlags & u16(CModelFlagsFlagBits::fDepthWrite) &&
                  MaterialFlags & u32(CCubeMaterialFlagBits::fDepthWrite)>,
      early_depth_stencil<(MaterialFlags & u16(CCubeMaterialFlagBits::fAlphaTest)) == 0>> {
  static constexpr bool AlphaTest = MaterialFlags & u16(CCubeMaterialFlagBits::fAlphaTest);
};

template <uint32_t NSkinSlots, uint32_t NCol, uint32_t NUv, uint32_t NWeight, BlendMaterial::ShaderType Type, EPostType Post,
          bool WorldShadow, class LightmapTraits, class DiffuseTraits,
          class EmissiveTraits, class SpecularTraits, class ExtendedSpecularTraits, class ReflectionTraits,
          class AlphaTraits, bool CubeReflection, MaterialBlendMode MaterialBlendMode, u32 MaterialFlags,
          u8 GameBlendMode, u16 ModelFlags, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
struct CModelShadersPipeline : CModelShadersPipelineConfig<MaterialBlendMode, MaterialFlags, GameBlendMode, ModelFlags,
                                                           ColorUpdate, AlphaUpdate, DstAlpha> {
  CModelShadersPipeline(hsh::uniform_buffer<CModelShaders::VertUniform<NSkinSlots>> vu,
                        hsh::uniform_buffer<CModelShaders::FragmentUniform> fragu HSH_VAR_STAGE(fragment),
                        hsh::uniform_buffer<std::array<CModelShaders::TCGMatrix, 8>> tcgu,
                        hsh::uniform_buffer<CModelShaders::ReflectMtx> refu, hsh::texture2d Lightmap,
                        hsh::texture2d Diffuse, hsh::texture2d Emissive, hsh::texture2d Specular,
                        hsh::texture2d ExtendedSpecular, hsh::texture2d Reflection, hsh::texture2d Alpha,
                        hsh::texture2d ReflectionIndTex, hsh::texture2d ExtTex0, hsh::texture2d ExtTex1,
                        hsh::texture2d ExtTex2, DynReflectionTexType<CubeReflection> dynReflection,
                        hsh::vertex_buffer<CModelShaders::VertData<NCol, NUv, NWeight>> vd) {
    hsh::float4 mvPos;
    hsh::float4 mvNorm;

    hsh::float4 objPos;
    hsh::float4 objNorm;
    if constexpr (NSkinSlots != 0) {
      objPos = hsh::float4(0.f);
      objNorm = hsh::float4(0.f);
      for (uint32_t i = 0; i < NSkinSlots; ++i) {
        objPos += (vu->objs[i] * hsh::float4(vd->posIn, 1.f)) * vd->weightIn[i / 4][i % 4];
        objNorm += (vu->objsInv[i] * hsh::float4(vd->normIn, 1.f)) * vd->weightIn[i / 4][i % 4];
      }
      objPos[3] = 1.f;
      objNorm = hsh::float4(hsh::normalize(objNorm.xyz()), 0.f);
      mvPos = vu->mv * objPos;
      mvNorm = hsh::float4(hsh::normalize((vu->mvInv * objNorm).xyz()), 0.f);
      this->position = vu->proj * mvPos;
    } else {
      objPos = hsh::float4(vd->posIn, 1.f);
      objNorm = hsh::float4(vd->normIn, 0.f);
      mvPos = vu->mv * objPos;
      mvNorm = vu->mvInv * objNorm;
      this->position = vu->proj * mvPos;
    }

    hsh::float2 LightmapUv = hsh::float2(0.f);
    hsh::float2 DiffuseUv = hsh::float2(0.f);
    hsh::float2 EmissiveUv = hsh::float2(0.f);
    hsh::float2 SpecularUv = hsh::float2(0.f);
    hsh::float2 ExtendedSpecularUv = hsh::float2(0.f);
    hsh::float2 ReflectionUv = hsh::float2(0.f);
    hsh::float2 AlphaUv = hsh::float2(0.f);
    hsh::float2 ShadowUv = hsh::float2(0.f);
    hsh::float2 DynReflectionUv = hsh::float2(0.f);
    hsh::float2 DynReflectionIndUv = hsh::float2(0.f);
    hsh::float2 ExtUv0 = hsh::float2(0.f);
    hsh::float2 ExtUv1 = hsh::float2(0.f);
    hsh::float2 ExtUv2 = hsh::float2(0.f);

#define EMIT_TCG(Pass)                                                                                                 \
  if constexpr (Pass##Traits::Source_ != MaterialTexCoordSource::Invalid) {                                            \
    if constexpr (Pass##Traits::MtxIdx_ >= 0) {                                                                        \
      hsh::float4 src;                                                                                                 \
      switch (Pass##Traits::Source_) {                                                                                 \
      case MaterialTexCoordSource::Position:                                                                           \
        src = hsh::float4(objPos.xyz(), 1.f);                                                                          \
        break;                                                                                                         \
      case MaterialTexCoordSource::Normal:                                                                             \
        src = hsh::float4(objNorm.xyz(), 1.f);                                                                         \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex0:                                                                               \
        src = hsh::float4(vd->uvIn[0], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex1:                                                                               \
        src = hsh::float4(vd->uvIn[1], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex2:                                                                               \
        src = hsh::float4(vd->uvIn[2], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex3:                                                                               \
        src = hsh::float4(vd->uvIn[3], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex4:                                                                               \
        src = hsh::float4(vd->uvIn[4], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex5:                                                                               \
        src = hsh::float4(vd->uvIn[5], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex6:                                                                               \
        src = hsh::float4(vd->uvIn[6], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex7:                                                                               \
        src = hsh::float4(vd->uvIn[7], 0.f, 1.f);                                                                      \
        break;                                                                                                         \
      case MaterialTexCoordSource::Invalid:                                                                            \
        break;                                                                                                         \
      }                                                                                                                \
      hsh::float3 tmp = ((*tcgu)[Pass##Traits::MtxIdx_].mtx * src).xyz();                                              \
      if constexpr (Pass##Traits::Normalize_)                                                                          \
        tmp = hsh::normalize(tmp);                                                                                     \
      hsh::float4 tmpProj = (*tcgu)[Pass##Traits::MtxIdx_].postMtx * hsh::float4(tmp, 1.f);                            \
      Pass##Uv = (tmpProj / tmpProj.w).xy();                                                                           \
    } else {                                                                                                           \
      switch (Pass##Traits::Source_) {                                                                                 \
      case MaterialTexCoordSource::Position:                                                                           \
        Pass##Uv = objPos.xy();                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Normal:                                                                             \
        Pass##Uv = objNorm.xy();                                                                                       \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex0:                                                                               \
        Pass##Uv = vd->uvIn[0];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex1:                                                                               \
        Pass##Uv = vd->uvIn[1];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex2:                                                                               \
        Pass##Uv = vd->uvIn[2];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex3:                                                                               \
        Pass##Uv = vd->uvIn[3];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex4:                                                                               \
        Pass##Uv = vd->uvIn[4];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex5:                                                                               \
        Pass##Uv = vd->uvIn[5];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex6:                                                                               \
        Pass##Uv = vd->uvIn[6];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Tex7:                                                                               \
        Pass##Uv = vd->uvIn[7];                                                                                        \
        break;                                                                                                         \
      case MaterialTexCoordSource::Invalid:                                                                            \
        break;                                                                                                         \
      }                                                                                                                \
    }                                                                                                                  \
  }
    EMIT_TCG(Lightmap)
    EMIT_TCG(Diffuse)
    EMIT_TCG(Emissive)
    EMIT_TCG(Specular)
    EMIT_TCG(ExtendedSpecular)
    EMIT_TCG(Reflection)
    EMIT_TCG(Alpha)

    if constexpr ((MaterialFlags & (u32(CCubeMaterialFlagBits::fSamusReflection) |
                                    u32(CCubeMaterialFlagBits::fSamusReflectionIndirectTexture))) != 0) {
      DynReflectionIndUv =
          hsh::normalize((refu->indMtx * hsh::float4(objPos.xyz(), 1.f)).xz()) * hsh::float2(0.5f) + hsh::float2(0.5f);
      DynReflectionUv = (refu->reflectMtx * hsh::float4(objPos.xyz(), 1.f)).xy();
    }

    hsh::float3 lighting;
    switch (Post) {
    case EPostType::ThermalHot:
    case EPostType::ThermalCold:
    case EPostType::Solid:
    case EPostType::MBShadow:
      lighting = hsh::float3(1.f);
      break;
    default:
      lighting = fragu->ambient.xyz();

      for (int i = 0; i < URDE_MAX_LIGHTS; ++i) {
        hsh::float3 delta = mvPos.xyz() - fragu->lights[i].pos;
        float dist = hsh::length(delta);
        hsh::float3 deltaNorm = delta / dist;
        float angDot = hsh::max(hsh::dot(deltaNorm, fragu->lights[i].dir), 0.f);
        float att = 1.f / (fragu->lights[i].linAtt[2] * dist * dist + fragu->lights[i].linAtt[1] * dist +
                           fragu->lights[i].linAtt[0]);
        float angAtt = fragu->lights[i].angAtt[2] * angDot * angDot + fragu->lights[i].angAtt[1] * angDot +
                       fragu->lights[i].angAtt[0];
        hsh::float3 thisColor =
            fragu->lights[i].color.xyz() * angAtt * att * hsh::max(hsh::dot(-deltaNorm, mvNorm.xyz()), 0.f);
        if (WorldShadow && i == 0)
          thisColor *= ExtTex0.sample<float>(ShadowUv, ClampSamp).x;
        lighting += thisColor;
      }

      lighting = hsh::saturate(lighting);
      break;
    }

    hsh::float3 DynReflectionSample HSH_VAR_STAGE(fragment);
    if constexpr ((MaterialFlags & u32(CCubeMaterialFlagBits::fSamusReflectionIndirectTexture)) != 0) {
      DynReflectionSample = dynReflection
                                .template sample<float>((ReflectionIndTex.sample<float>(DynReflectionIndUv, Samp).xw() -
                                                         hsh::float2(0.5f)) *
                                                        hsh::float2(0.5f) +
                                                        DynReflectionUv,
                                                        ReflectSamp)
                                .xyz() *
                            refu->reflectAlpha;
    } else if constexpr ((MaterialFlags & u32(CCubeMaterialFlagBits::fSamusReflection)) != 0) {
      DynReflectionSample =
          dynReflection.template sample<float>(DynReflectionUv, ReflectSamp).xyz() * refu->reflectAlpha;
    } else {
      DynReflectionSample = hsh::float3(0.f);
    }

    const hsh::float3 kRGBToYPrime = hsh::float3(0.257f, 0.504f, 0.098f);

#define Sample(Pass)                                                                                                   \
  (Pass##Traits::SampleAlpha_                                                                                          \
       ? (Pass##Traits::ConstantColor_::Constant ? hsh::float3(Pass##Traits::ConstantColor_::A_)                       \
                                                 : hsh::float3(Pass.sample<float>(Pass##Uv, Samp).w))                  \
       : (Pass##Traits::ConstantColor_::Constant ? Pass##Traits::ConstantColor_::RGB                                   \
                                                 : Pass.sample<float>(Pass##Uv, Samp).xyz()))
#define SampleAlpha(Pass)                                                                                              \
  (Pass##Traits::SampleAlpha_                                                                                          \
       ? (Pass##Traits::ConstantColor_::Constant ? Pass##Traits::ConstantColor_::A_                                    \
                                                 : Pass.sample<float>(Pass##Uv, Samp).w)                               \
       : (Pass##Traits::ConstantColor_::Constant ? hsh::dot(Pass##Traits::ConstantColor_::RGB, kRGBToYPrime)           \
                                                 : hsh::dot(Pass.sample<float>(Pass##Uv, Samp).xyz(), kRGBToYPrime)))

    switch (Type) {
    case BlendMaterial::ShaderType::Invalid:
      this->color_out[0] = hsh::float4(Sample(Diffuse), SampleAlpha(Alpha));
      break;
    case BlendMaterial::ShaderType::RetroShader:
      this->color_out[0] = hsh::float4(
          (Sample(Lightmap) * fragu->lightmapMul.xyz() + lighting) * Sample(Diffuse) + Sample(Emissive) +
              (Sample(Specular) + Sample(ExtendedSpecular) * lighting) * Sample(Reflection) + DynReflectionSample,
          SampleAlpha(Alpha));
      break;
    case BlendMaterial::ShaderType::RetroDynamicShader:
      this->color_out[0] = hsh::float4(
          (Sample(Lightmap) * fragu->lightmapMul.xyz() + lighting) * (Sample(Diffuse) + Sample(Emissive)) *
                  fragu->lightmapMul.xyz() +
              (Sample(Specular) + Sample(ExtendedSpecular) * lighting) * Sample(Reflection) + DynReflectionSample,
          SampleAlpha(Alpha));
      break;
    case BlendMaterial::ShaderType::RetroDynamicAlphaShader:
      this->color_out[0] = hsh::float4(
          (Sample(Lightmap) * fragu->lightmapMul.xyz() + lighting) * (Sample(Diffuse) + Sample(Emissive)) *
                  fragu->lightmapMul.xyz() +
              (Sample(Specular) + Sample(ExtendedSpecular) * lighting) * Sample(Reflection) + DynReflectionSample,
          SampleAlpha(Alpha) * fragu->lightmapMul.w);
      break;
    case BlendMaterial::ShaderType::RetroDynamicCharacterShader:
      this->color_out[0] = hsh::float4(
          (Sample(Lightmap) + lighting) * Sample(Diffuse) + Sample(Emissive) * fragu->lightmapMul.xyz() +
              (Sample(Specular) + Sample(ExtendedSpecular) * lighting) * Sample(Reflection) + DynReflectionSample,
          SampleAlpha(Alpha));
      break;
    }

    FOG_SHADER(fragu->fog)

    if constexpr (Post == EPostType::Normal) {
      if constexpr (this->DstColorBlendFactor<0> == hsh::One)
        this->color_out[0] =
            hsh::float4(hsh::lerp(this->color_out[0], hsh::float4(0.f), fogZ).xyz(), this->color_out[0].w);
      else
        this->color_out[0] =
            hsh::float4(hsh::lerp(this->color_out[0], fragu->fog.m_color, fogZ).xyz(), this->color_out[0].w);
      if constexpr (GameBlendMode == 2) {
        if (this->DstColorBlendFactor<0> != hsh::One)
          this->color_out[0] += fragu->flagsColor;
      } else if constexpr (GameBlendMode != 0) {
        this->color_out[0] *= fragu->flagsColor;
      }
    } else if constexpr (Post == EPostType::ThermalHot) {
      this->color_out[0] = hsh::float4(ExtTex0.sample<float>(ExtUv0, Samp).x) * fragu->flagsColor + fragu->ambient;
    } else if constexpr (Post == EPostType::ThermalCold) {
      this->color_out[0] *= hsh::float4(0.75f);
    } else if constexpr (Post == EPostType::Solid) {
      this->color_out[0] = fragu->flagsColor;
    } else if constexpr (Post == EPostType::MBShadow) {
      float idTexel = ExtTex0.sample<float>(ExtUv0, Samp).w;
      float sphereTexel = ExtTex1.sample<float>(ExtUv1, ClampEdgeSamp).x;
      float fadeTexel = ExtTex2.sample<float>(ExtUv2, ClampEdgeSamp).w;
      float val = ((hsh::abs(idTexel - fragu->ambient.x) < 0.001f)
                       ? (hsh::dot(mvNorm.xyz(), fragu->flagsColor.xyz()) * fragu->flagsColor.w)
                       : 0.f) *
                  sphereTexel * fadeTexel;
      this->color_out[0] = hsh::float4(0.f, 0.f, 0.f, val);
    } else if constexpr (Post == EPostType::Disintegrate) {
      hsh::float4 texel0 = ExtTex0.sample<float>(ExtUv0, Samp);
      hsh::float4 texel1 = ExtTex0.sample<float>(ExtUv1, Samp);
      this->color_out[0] = hsh::lerp(hsh::float4(0.f), texel1, texel0);
      this->color_out[0] = hsh::float4(fragu->flagsColor.xyz() + this->color_out[0].xyz(), this->color_out[0].w);
      if constexpr (this->DstColorBlendFactor<0> == hsh::One)
        this->color_out[0] =
            hsh::float4(hsh::lerp(this->color_out[0], hsh::float4(0.f), fogZ).xyz(), this->color_out[0].w);
      else
        this->color_out[0] =
            hsh::float4(hsh::lerp(this->color_out[0], fragu->fog.m_color, fogZ).xyz(), this->color_out[0].w);
    }

    if (this->AlphaTest && this->color_out[0].w < 0.25f)
      hsh::discard();
  }
};

#if 0
template <uint32_t NSkinSlots, uint32_t NCol, uint32_t NUv, uint32_t NWeight, EShaderType Type, EPostType Post,
          bool WorldShadow, class LightmapTraits, class DiffuseTraits,
          class EmissiveTraits, class SpecularTraits, class ExtendedSpecularTraits, class ReflectionTraits,
          class AlphaTraits, bool CubeReflection, MaterialBlendMode MaterialBlendMode, u32 MaterialFlags,
          u8 GameBlendMode, u16 ModelFlags, bool ColorUpdate, bool AlphaUpdate, bool DstAlpha>
#endif

hsh::binding& CModelShaders::SetCurrent(const CModelFlags& modelFlags, const CBooSurface& surface, const CBooModel& model) {
  const auto& material = model.x4_matSet->materials[surface.m_data.matIdx];
  const auto& vtxFmt = model.m_vtxFmt;
  material.chunks
  m_dataBind.hsh_bind(CModelShadersPipeline<vtxFmt.NSkinSlots, vtxFmt.NCol, vtxFmt.NUv, vtxFmt.NWeight, material.shaderType,
      modelFlags.m_postType, CBooModel::g_shadowMap, >());
  return m_dataBind;
}

struct CCubeMaterial {
  const u8* x0_data;
  static u32 sReflectionType;
  static const CCubeModel* sLastModelCached;
  static const CCubeModel* sRenderingModel;
  static float gReflectionAlpha;
#if 0
  CCubeMaterialFlags flags;
  u32 texCount;
  // tex indices here
  CCubeMaterialVatFlags vatFlags;
  u32 groupIndex;
  u32 kColorCount;
  // kcolors here
  u16 destBlendFactor;
  u16 srcBlendFactor;
  u32 reflectionIndTexIdx;
  u32 colorChanCount;
  // cc here
  u32 tevStageCount;
#endif

  void SetCurrentBlack() const {
    u32 texCount = *reinterpret_cast<const u32*>(x0_data + 4);
    CCubeMaterialFlags flags = *reinterpret_cast<const CCubeMaterialFlags*>(x0_data);
    CCubeMaterialVatFlags vatFlags = *reinterpret_cast<const CCubeMaterialVatFlags*>(x0_data + 8 + 4 * texCount);

    if (flags & (CCubeMaterialFlags(CCubeMaterialFlagBits::fDepthSorting) |
                 CCubeMaterialFlags(CCubeMaterialFlagBits::fAlphaTest))) {}
  }

  static void EnsureViewDepStateCached(const CCubeSurface* surf) {}

  static void SetupBlendMode(u32 blendFactors, const CModelFlags& modelFlags, bool alphaTest) {
    u16 newSrcFactor = blendFactors & 0xffff;
    u16 newDstFactor = blendFactors >> 16 & 0xffff;
    if (alphaTest) {
      // GXSetAlphaCompare(6, 64, 1, 0, 0);
      // GXSetZCompLoc(0);
      newSrcFactor = 1;
      newDstFactor = 0;
    } else {
      // GXSetAlphaCompare(7, 0, 1, 7, 0);
      // GXSetZCompLoc(1);
    }

    if (modelFlags.x0_blendMode > 4) {
      if (newSrcFactor == 1) {
        newSrcFactor = 4;
        if (newDstFactor == 0) {
          newDstFactor = modelFlags.x0_blendMode > 6 ? 1 : 5;
        }
      }
    }

    // set fog color zero if dst blend zero
    // set blend factors
  }

  static void HandleDepth(u16 modelFlags, CCubeMaterialFlags materialFlags) {
    u32 compare = 0;
    if ((modelFlags & 0x1) == 0) {
      compare = 7; // ALWAYS
    } else {
      if ((modelFlags & 0x8) != 0) {
        compare = (modelFlags & 0x10) != 0 ? 4 : 6; // GREATER or GEQUAL
      } else {
        compare = (modelFlags & 0x10) != 0 ? 1 : 3; // LESS or LEQUAL
      }
    }
    bool depthWrite = ((modelFlags & 0x2) == 0x2 && materialFlags & CCubeMaterialFlagBits::fDepthWrite);
  }

  static u32 HandleColorChannels(u32 chanCount, u32 firstChan) { return 0; }

  static void HandleTev(u32 tevCur, const u8* materialDataCur, const u32* texMapTexCoordFlags, bool shadowMapsEnabled) {

  }

  static u32 HandleAnimatedUV(const u32* uvAnim, u32 texMtx, u32 pttTexMtx) { return 0; }

  static void HandleTransparency(u32& finalTevCount, u32& finalKColorCount, const CModelFlags& modelFlags,
                                 u32 blendFactors, u32& finalCCFlags, u32& finalACFlags) {
    if (modelFlags.x0_blendMode == 2) {
      u16 dstFactor = blendFactors >> 16 & 0xffff;
      if (dstFactor == 1)
        return;
    }
    if (modelFlags.x0_blendMode == 3) {
      // Stage outputting splatted KAlpha as color to reg0
      // GXSetTevColorIn(finalTevCount, TEVCOLORARG_ZERO, TEVCOLORARG_ZERO, TEVCOLORARG_ZERO, TEVCOLORARG_KONST);
      // GXSetTevAlphaIn(finalTevCount, TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_APREV);
      // GXSetTevColorOp(finalTevCount, 0, 0, 0, 1, 1); // ColorReg0
      // GXSetTevKColorSel(finalTevCount, finalKColorCount+28);
      // GXSetTevAlphaOp(finalTevCount, 0, 0, 0, 1, 0); // AlphaRegPrev
      // GXSetTevOrder(finalTevCount, 255, 255, 255);
      // GXSetTevDirect(finalTevCount);
      // Stage interpolating from splatted KAlpha using KColor
      // GXSetTevColorIn(finalTevCount + 1, TEVCOLORARG_CPREV, TEVCOLORARG_C0, TEVCOLORARG_KONST, TEVCOLORARG_ZERO);
      // GXSetTevAlphaIn(finalTevCount + 1, TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_APREV);
      // GXSetTevKColorSel(finalTevCount, finalKColorCount+12);
      // SetStandardTevColorAlphaOp(finalTevCount + 1);
      // GXSetTevDirect(finalTevCount + 1);
      // GXSetTevOrder(finalTevCount + 1, 255, 255, 255);
      // GXSetTevKColor(finalKColorCount, modelFlags.x4_color);
      finalKColorCount += 1;
      finalTevCount += 2;
    } else {
      // Mul KAlpha
      u32 tevAlpha = 0x000380C7; // TEVALPHAARG_ZERO, TEVALPHAARG_KONST, TEVALPHAARG_APREV, TEVALPHAARG_ZERO
      if (modelFlags.x0_blendMode == 8) {
        // Set KAlpha
        tevAlpha = 0x00031CE7; // TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_ZERO, TEVALPHAARG_KONST
      }
      // Mul KColor
      u32 tevColor = 0x000781CF; // TEVCOLORARG_ZERO, TEVCOLORARG_KONST, TEVCOLORARG_CPREV, TEVCOLORARG_ZERO
      if (modelFlags.x0_blendMode == 2) {
        // Add KColor
        tevColor = 0x0007018F; // TEVCOLORARG_ZERO, TEVCOLORARG_ONE, TEVCOLORARG_CPREV, TEVCOLORARG_KONST
      }
      // GXSetTevColorIn(finalTevCount)
      // GXSetTevAlphaIn(finalTevCount)
      // SetStandardTevColorAlphaOp(finalTevCount);
      finalCCFlags = 0x100; // Just clamp, output prev reg
      finalACFlags = 0x100;
      // GXSetTevDirect(finalTevCount);
      // GXSetTevOrder(finalTevCount, 255, 255, 255);
      // GXSetTevKColor(finalKColorCount, modelFlags.x4_color);
      // GXSetTevKColorSel(finalTevCount, finalKColorCount+12);
      // GXSetTevKAlphaSel(finalTevCount, finalKColorCount+28);
      finalTevCount += 1;
      finalKColorCount += 1;
    }
  }

  static u32 HandleReflection(bool usesTevReg2, u32 indTexSlot, u32 r5, u32 finalTevCount, u32 texCount, u32 tcgCount,
                              u32 finalKColorCount, u32& finalCCFlags, u32& finalACFlags) {
    return 0;
  }

  static void DoModelShadow(u32 texCount, u32 tcgCount) {}

  static void DoPassthru(u32 finalTevCount) {}

  void SetCurrent(const CModelFlags& modelFlags, const CCubeSurface& surface, const CCubeModel& model) const {
    if (sLastMaterialCached == x0_data) {
      switch (sReflectionType) {
      case 2:
        break;
      default:
        return;
      case 1:
        if (sLastModelCached == sRenderingModel)
          return;
        break;
      }
    }

    if (sRenderModelBlack) {
      SetCurrentBlack();
      return;
    }

    u32 numIndStages = 0;

    CCubeMaterialFlags flags = *reinterpret_cast<const CCubeMaterialFlags*>(x0_data);
    const u8* materialDataCur = x0_data;

    bool reflection = bool(flags & (CCubeMaterialFlags(CCubeMaterialFlagBits::fSamusReflectionSurfaceEye) |
                                    CCubeMaterialFlags(CCubeMaterialFlagBits::fSamusReflection)));
    if (reflection) {
      sLastMaterialCached = x0_data;
      sRenderingModel = &model;
      EnsureViewDepStateCached(flags & CCubeMaterialFlagBits::fSamusReflectionSurfaceEye ? &surface : nullptr);
    }

    sRenderingModel = &model;
    sLastMaterialCached = x0_data;

    u32 texCount = *reinterpret_cast<const u32*>(materialDataCur + 4);

    if ((modelFlags.x2_flags & 0x4) == 0) {
      materialDataCur += 8;
      for (u32 i = 0; i < texCount; ++i) {
        u32 texIdx = *reinterpret_cast<const u32*>(materialDataCur);
        sRenderingModel->x1c_texs[texIdx].x8_tex->Load(i, CTexture::EClampMode::One);
        materialDataCur += 4;
      }
    } else {
      materialDataCur += (2 + texCount) * 4;
    }

    u32 groupIdx = *reinterpret_cast<const u32*>(materialDataCur + 4);
    if (sLastMaterialUnique != UINT32_MAX && groupIdx == UINT32_MAX && sReflectionType == 0)
      return;
    sLastMaterialUnique = groupIdx;

    CCubeMaterialVatFlags vatFlags = *reinterpret_cast<const CCubeMaterialVatFlags*>(materialDataCur);
    // SetVtxDescv_Compressed(vatFlags);
    materialDataCur += 8;

    if (bool(flags & CCubeMaterialFlagBits::fLightmapUvArray) != CCubeModel::sUsingPackedLightmaps) {
      model.SetUsingPackedLightmaps();
    }

    u32 finalKColorCount = 0;
    if (flags & CCubeMaterialFlagBits::fKonstValues) {
      u32 konstCount = *reinterpret_cast<const u32*>(materialDataCur);
      finalKColorCount = konstCount;
      materialDataCur += 4;
      for (u32 i = 0; i < konstCount; ++i) {
        u32 kColor = *reinterpret_cast<const u32*>(materialDataCur);
        materialDataCur += 4;
        // Set KColor
      }
    }

    u32 blendFactors = *reinterpret_cast<const u32*>(materialDataCur);
    materialDataCur += 4;

    if (g_Renderer->IsInAreaDraw()) {
      // Blackout fog, additive blend
    } else {
      SetupBlendMode(blendFactors, modelFlags, bool(flags & CCubeMaterialFlagBits::fAlphaTest));
    }

    bool indTex = bool(flags & CCubeMaterialFlagBits::fSamusReflectionIndirectTexture);
    u32 indTexSlot = 0;
    if (indTex) {
      indTexSlot = *reinterpret_cast<const u32*>(materialDataCur);
      materialDataCur += 4;
    }

    HandleDepth(modelFlags.x2_flags, flags);

    u32 chanCount = *reinterpret_cast<const u32*>(materialDataCur);
    materialDataCur += 4;
    u32 firstChan = *reinterpret_cast<const u32*>(materialDataCur);
    materialDataCur += 4 * chanCount;
    u32 finalNumColorChans = HandleColorChannels(chanCount, firstChan);

    u32 firstTev = 0;
    if (kShadowMapsEnabled)
      firstTev = 2;

    u32 matTevCount = *reinterpret_cast<const u32*>(materialDataCur);
    materialDataCur += 4;
    u32 finalTevCount = matTevCount;

    const u32* texMapTexCoordFlags = reinterpret_cast<const u32*>(materialDataCur + matTevCount * 20);
    const u32* tcgs = reinterpret_cast<const u32*>(texMapTexCoordFlags + matTevCount);
    bool usesTevReg2 = false;

    u32 finalCCFlags = 0;
    u32 finalACFlags = 0;

    if (g_Renderer->IsThermalVisorActive()) {
      finalTevCount = firstTev + 1;
      u32 ccFlags = *reinterpret_cast<const u32*>(materialDataCur + 8);
      finalCCFlags = ccFlags;
      u32 outputReg = ccFlags >> 9 & 0x3;
      if (outputReg == 1) { // TevReg0
        materialDataCur += 20;
        texMapTexCoordFlags += 1;
        finalCCFlags = *reinterpret_cast<const u32*>(materialDataCur + 8);
        // Set TevReg0 = 0xc0c0c0c0
      }
      finalACFlags = *reinterpret_cast<const u32*>(materialDataCur + 12);
      HandleTev(firstTev, materialDataCur, texMapTexCoordFlags, kShadowMapsEnabled);
      usesTevReg2 = false;
    } else {
      finalTevCount = firstTev + matTevCount;
      for (u32 i = firstTev; i < finalTevCount; ++i) {
        HandleTev(i, materialDataCur, texMapTexCoordFlags, kShadowMapsEnabled && i == firstTev);
        u32 ccFlags = *reinterpret_cast<const u32*>(materialDataCur + 8);
        finalCCFlags = ccFlags;
        finalACFlags = *reinterpret_cast<const u32*>(materialDataCur + 12);
        u32 outputReg = ccFlags >> 9 & 0x3;
        if (outputReg == 3) { // TevReg2
          usesTevReg2 = true;
        }
        materialDataCur += 20;
        texMapTexCoordFlags += 1;
      }
    }

    u32 tcgCount = 0;
    if (g_Renderer->IsThermalVisorActive()) {
      u32 fullTcgCount = *tcgs;
      tcgCount = std::min(fullTcgCount, 2u);
      for (u32 i = 0; i < tcgCount; ++i) {
        // Set TCG
      }
      tcgs += fullTcgCount + 1;
    } else {
      tcgCount = *tcgs;
      for (u32 i = 0; i < tcgCount; ++i) {
        // Set TCG
      }
      tcgs += tcgCount + 1;
    }

    const u32* uvAnim = tcgs;
    u32 animCount = uvAnim[1];
    uvAnim += 2;
    u32 texMtx = 30;
    u32 pttTexMtx = 64;
    for (u32 i = 0; i < animCount; ++i) {
      u32 size = HandleAnimatedUV(uvAnim, texMtx, pttTexMtx);
      if (size == 0)
        break;
      uvAnim += size;
      texMtx += 3;
      pttTexMtx += 3;
    }

    if (modelFlags.x0_blendMode) {
      HandleTransparency(finalTevCount, finalKColorCount, modelFlags, blendFactors, finalCCFlags, finalACFlags);
    }

    if (reflection) {
      if (gReflectionAlpha > 0.f) {
        u32 additionalTevs = 0;
        if (indTex) {
          additionalTevs = HandleReflection(usesTevReg2, indTexSlot, 0, finalTevCount, texCount, tcgCount,
                                            finalKColorCount, finalCCFlags, finalACFlags);
          numIndStages = 1;
          tcgCount += 2;
        } else {
          additionalTevs = HandleReflection(usesTevReg2, 255, 0, finalTevCount, texCount, tcgCount, finalKColorCount,
                                            finalCCFlags, finalACFlags);
          tcgCount += 1;
        }
        texCount += 1;
        finalTevCount += additionalTevs;
        finalKColorCount += 1;
      } else if (((finalCCFlags >> 9) & 0x3) != 0) {
        DoPassthru(finalTevCount);
        finalTevCount += 1;
      }
    }

    if (kShadowMapsEnabled) {
      DoModelShadow(texCount, tcgCount);
      tcgCount += 1;
    }

    // SetNumIndStages(numIndStages);
    // SetNumTevStages(finalTevCount);
    // SetNumTexGens(tcgCount);
    // SetNumColorChans(finalNumColorChans);
  }
};

} // namespace urde
