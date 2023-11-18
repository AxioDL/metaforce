#pragma once

#include "Runtime/ConsoleVariables/CVar.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CTevCombiners.hpp"
#include "Runtime/Graphics/GX.hpp"
#include "Runtime/RetroTypes.hpp"

#include <array>
#include <chrono>
#include <vector>

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector2i.hpp>

#include <aurora/aurora.h>
#include <optick.h>

using frame_clock = std::chrono::high_resolution_clock;

namespace metaforce {
class CTexture;
extern CVar* g_disableLighting;
class CTimeProvider;

enum class ERglCullMode : std::underlying_type_t<GXCullMode> {
  None = GX_CULL_NONE,
  Front = GX_CULL_FRONT,
  Back = GX_CULL_BACK,
  All = GX_CULL_ALL,
};

enum class ERglBlendMode : std::underlying_type_t<GXBlendMode> {
  None = GX_BM_NONE,
  Blend = GX_BM_BLEND,
  Logic = GX_BM_LOGIC,
  Subtract = GX_BM_SUBTRACT,
  Max = GX_MAX_BLENDMODE,
};

enum class ERglBlendFactor : std::underlying_type_t<GXBlendFactor> {
  Zero = GX_BL_ZERO,
  One = GX_BL_ONE,
  SrcColor = GX_BL_SRCCLR,
  InvSrcColor = GX_BL_INVSRCCLR,
  SrcAlpha = GX_BL_SRCALPHA,
  InvSrcAlpha = GX_BL_INVSRCALPHA,
  DstAlpha = GX_BL_DSTALPHA,
  InvDstAlpha = GX_BL_INVDSTALPHA,
  DstColor = GX_BL_DSTCLR,
  InvDstColor = GX_BL_INVDSTCLR,
};

enum class ERglLogicOp : std::underlying_type_t<GXLogicOp> {
  Clear = GX_LO_CLEAR,
  And = GX_LO_AND,
  RevAnd = GX_LO_REVAND,
  Copy = GX_LO_COPY,
  InvAnd = GX_LO_INVAND,
  NoOp = GX_LO_NOOP,
  Xor = GX_LO_XOR,
  Or = GX_LO_OR,
  Nor = GX_LO_NOR,
  Equiv = GX_LO_EQUIV,
  Inv = GX_LO_INV,
  RevOr = GX_LO_REVOR,
  InvCopy = GX_LO_INVCOPY,
  InvOr = GX_LO_INVOR,
  NAnd = GX_LO_NAND,
  Set = GX_LO_SET,
};

enum class ERglAlphaFunc : std::underlying_type_t<GXCompare> {
  Never = GX_NEVER,
  Less = GX_LESS,
  Equal = GX_EQUAL,
  LEqual = GX_LEQUAL,
  Greater = GX_GREATER,
  NEqual = GX_NEQUAL,
  GEqual = GX_GEQUAL,
  Always = GX_ALWAYS,
};

enum class ERglAlphaOp : std::underlying_type_t<GXAlphaOp> {
  And = GX_AOP_AND,
  Or = GX_AOP_OR,
  Xor = GX_AOP_XOR,
  XNor = GX_AOP_XNOR,
  Max = GX_MAX_ALPHAOP,
};

enum class ERglEnum : std::underlying_type_t<GXCompare> {
  Never = GX_NEVER,
  Less = GX_LESS,
  Equal = GX_EQUAL,
  LEqual = GX_LEQUAL,
  Greater = GX_GREATER,
  NEqual = GX_NEQUAL,
  GEqual = GX_GEQUAL,
  Always = GX_ALWAYS,
};

using ERglLight = u8;

enum class ERglTexOffset : std::underlying_type_t<GXTexOffset> {
  Zero = GX_TO_ZERO,
  Sixteenth = GX_TO_SIXTEENTH,
  Eighth = GX_TO_EIGHTH,
  Fourth = GX_TO_FOURTH,
  Half = GX_TO_HALF,
  One = GX_TO_ONE,
};

enum class ERglFogMode : std::underlying_type_t<GXFogType> {
  None = GX_FOG_NONE,

  PerspLin = GX_FOG_PERSP_LIN,
  PerspExp = GX_FOG_PERSP_EXP,
  PerspExp2 = GX_FOG_ORTHO_EXP2,
  PerspRevExp = GX_FOG_PERSP_REVEXP,
  PerspRevExp2 = GX_FOG_PERSP_REVEXP2,

  OrthoLin = GX_FOG_ORTHO_LIN,
  OrthoExp = GX_FOG_ORTHO_EXP,
  OrthoExp2 = GX_FOG_ORTHO_EXP2,
  OrthoRevExp = GX_FOG_ORTHO_REVEXP,
  OrthoRevExp2 = GX_FOG_ORTHO_REVEXP2,
};

struct SViewport {
  u32 x0_left;
  u32 x4_top;
  u32 x8_width;
  u32 xc_height;
  float x10_halfWidth;
  float x14_halfHeight;
  float aspect;
};

struct SClipScreenRect {
  bool x0_valid = false;
  int x4_left = 0;
  int x8_top = 0;
  int xc_width = 0;
  int x10_height = 0;
  int x14_dstWidth = 0;
  float x18_uvXMin = 0.f;
  float x1c_uvXMax = 0.f;
  float x20_uvYMin = 0.f;
  float x24_uvYMax = 0.f;

  SClipScreenRect() = default;
  SClipScreenRect(bool valid, int left, int top, int width, int height, int dstWidth, float uvXMin, float uvXMax,
                  float uvYMin, float uvYMax)
  : x0_valid(valid)
  , x4_left(left)
  , x8_top(top)
  , xc_width(width)
  , x10_height(height)
  , x14_dstWidth(dstWidth)
  , x18_uvXMin(uvXMin)
  , x1c_uvXMax(uvXMax)
  , x20_uvYMin(uvYMin)
  , x24_uvYMax(uvYMax) {}

  SClipScreenRect(const SViewport& vp) {
    x4_left = vp.x0_left;
    x8_top = vp.x4_top;
    xc_width = vp.x8_width;
    x10_height = vp.xc_height;
  }
};

#define DEPTH_FAR 1.f
#define DEPTH_SKY 0.999f
#define DEPTH_TARGET_MANAGER 0.12500012f
#define DEPTH_WORLD (1.f / 8.f)
#define DEPTH_GUN (1.f / 32.f)
#define DEPTH_SCREEN_ACTORS (1.f / 64.f)
#define DEPTH_HUD (1.f / 512.f)
#define DEPTH_NEAR 0.f
#define CUBEMAP_RES 256
#define CUBEMAP_MIPS 6

class CGraphics {
public:
  struct CProjectionState {
    bool x0_persp;
    float x4_left;
    float x8_right;
    float xc_top;
    float x10_bottom;
    float x14_near;
    float x18_far;
  };

  static CProjectionState g_Proj;
  static zeus::CVector2f g_CachedDepthRange;
  // static CFogState g_Fog;
  static SViewport g_Viewport;
  static float g_ProjAspect;
  static u32 g_NumBreakpointsWaiting;
  static u32 g_FlippingState;
  static bool g_LastFrameUsedAbove;
  static bool g_InterruptLastFrameUsedAbove;
  static GX::LightMask g_LightActive;
  static std::array<GXLightObj, GX::MaxLights> g_LightObjs;
  static std::array<ELightType, GX::MaxLights> g_LightTypes;
  static zeus::CTransform g_GXModelView;
  static zeus::CTransform g_GXModelViewInvXpose;
  static zeus::CTransform g_GXModelMatrix;
  static zeus::CTransform g_ViewMatrix;
  static zeus::CVector3f g_ViewPoint;
  static zeus::CTransform g_GXViewPointMatrix;
  static zeus::CTransform g_CameraMatrix;
  static SClipScreenRect g_CroppedViewport;
  static bool g_IsGXModelMatrixIdentity;
  static zeus::CColor g_ClearColor;
  static float g_ClearDepthValue; // Was a 24bit value, we use a float range from [0,1]
  static bool g_IsBeginSceneClearFb;

  static ERglEnum g_depthFunc;
  static ERglCullMode g_cullMode;

  static void Startup();
  static void InitGraphicsVariables();
  static void InitGraphicsDefaults();
  static void SetDefaultVtxAttrFmt();
  static void DisableAllLights();
  static void LoadLight(ERglLight light, const CLight& info);
  static void EnableLight(ERglLight light);
  static void SetLightState(GX::LightMask lightState);
  static void SetAmbientColor(const zeus::CColor& col);
  static void SetFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color);
  static void SetDepthWriteMode(bool test, ERglEnum comp, bool write);
  static void SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp);
  static void SetCullMode(ERglCullMode);
  static void BeginScene();
  static void EndScene();
  static void Render2D(CTexture& tex, u32 x, u32 y, u32 w, u32 h, const zeus::CColor& col);
  static bool BeginRender2D(const CTexture& tex);
  static void DoRender2D(const CTexture& tex, s32 x, s32 y, s32 w1, s32 w2, s32 w3, s32 w4, s32 w5,
                         const zeus::CColor& col);
  static void EndRender2D(bool v);
  static void SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1);
  static void SetViewPointMatrix(const zeus::CTransform& xf);
  static void SetViewMatrix();
  static void SetModelMatrix(const zeus::CTransform& xf);
  static zeus::CMatrix4f CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar);
  static zeus::CMatrix4f GetPerspectiveProjectionMatrix();
  static const CProjectionState& GetProjectionState();
  static void SetProjectionState(const CProjectionState&);
  static void SetPerspective(float fovy, float aspect, float znear, float zfar);
  static void SetOrtho(float left, float right, float top, float bottom, float znear, float zfar);
  static void FlushProjection();
  static zeus::CVector2i ProjectPoint(const zeus::CVector3f& point);
  static SClipScreenRect ClipScreenRectFromMS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
  static SClipScreenRect ClipScreenRectFromVS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);

  static void SetViewportResolution(const zeus::CVector2i& res);
  static void SetViewport(int leftOff, int bottomOff, int width, int height);
  static void SetScissor(int leftOff, int bottomOff, int width, int height);
  static void SetDepthRange(float near, float far);

  static CTimeProvider* g_ExternalTimeProvider;
  static float g_DefaultSeconds;
  static u32 g_RenderTimings;
  static void SetExternalTimeProvider(CTimeProvider* provider) { g_ExternalTimeProvider = provider; }
  static float GetSecondsMod900();
  static void TickRenderTimings();
  static u32 g_FrameCounter;
  static u32 g_Framerate;
  static u32 g_FramesPast;
  static frame_clock::time_point g_FrameStartTime;
  static u32 GetFrameCounter() { return g_FrameCounter; }
  static u32 GetFPS() { return g_Framerate; }
  static void UpdateFPSCounter();
  static void SetUseVideoFilter(bool);
  static void SetClearColor(const zeus::CColor& color);
  static void SetCopyClear(const zeus::CColor& color, float depth);
  static void SetIsBeginSceneClearFb(bool clear);
  static u32 GetViewportLeft() { return g_Viewport.x0_left; }
  static u32 GetViewportTop() { return g_Viewport.x4_top; }
  static u32 GetViewportWidth() { return g_Viewport.x8_width; }
  static u32 GetViewportHeight() { return g_Viewport.xc_height; }
  static float GetViewportHalfWidth() { return g_Viewport.x10_halfWidth; }
  static float GetViewportHalfHeight() { return g_Viewport.x14_halfHeight; }
  static float GetViewportAspect() { return g_Viewport.aspect; }
  static bool IsCroppedViewportValid() { return g_CroppedViewport.x0_valid; }
  static int GetCroppedViewportLeft() { return g_CroppedViewport.x4_left; }
  static int GetCroppedViewportTop() { return g_CroppedViewport.x8_top; }
  static int GetCroppedViewportWidth() { return g_CroppedViewport.xc_width; }
  static int GetCroppedViewportHeight() { return g_CroppedViewport.x10_height; }
  static float GetCroppedViewportDstWidth() { return g_CroppedViewport.x14_dstWidth; }
  static float GetCroppedViewportUVXMin() { return g_CroppedViewport.x18_uvXMin; }
  static float GetCroppedViewportUVXMax() { return g_CroppedViewport.x1c_uvXMax; }
  static float GetCroppedViewportUVYMin() { return g_CroppedViewport.x20_uvYMin; }
  static float GetCroppedViewportUVYMax() { return g_CroppedViewport.x24_uvYMax; }

  static const std::array<zeus::CMatrix3f, 6> skCubeBasisMats;
  static u8 sSpareTextureData[];

  static void LoadDolphinSpareTexture(int width, int height, GXTexFmt format, void* data, GXTexMapID id);
  static void LoadDolphinSpareTexture(int width, int height, GXCITexFmt format, GXTlut tlut, void* data, GXTexMapID id);

  static void ResetGfxStates() noexcept;
  static void SetTevStates(u32 flags) noexcept;
  static void SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass);
  static void StreamBegin(GXPrimitive primitive);
  static void StreamNormal(const zeus::CVector3f& nrm);
  static void StreamColor(const zeus::CColor& color);
  static inline void StreamColor(float r, float g, float b, float a) { StreamColor({r, g, b, a}); }
  static void StreamTexcoord(const zeus::CVector2f& uv);
  static inline void StreamTexcoord(float x, float y) { StreamTexcoord({x, y}); }
  static void StreamVertex(const zeus::CVector3f& pos);
  static inline void StreamVertex(float xyz) { StreamVertex({xyz, xyz, xyz}); }
  static inline void StreamVertex(float x, float y, float z) { StreamVertex({x, y, z}); }
  static void StreamEnd();
  static void UpdateVertexDataStream();
  static void ResetVertexDataStream(bool end);
  static void FlushStream();
  static void FullRender();
  static void DrawPrimitive(GXPrimitive primitive, const zeus::CVector3f* pos, const zeus::CVector3f& normal,
                            const zeus::CColor& col, s32 numVerts);
  static void SetLineWidth(float width, ERglTexOffset offs);
};

template <class VTX>
class TriFanToStrip {
  std::vector<VTX>& m_vec;
  size_t m_start;
  size_t m_added = 0;

public:
  explicit TriFanToStrip(std::vector<VTX>& vec) : m_vec(vec), m_start(vec.size()) {}

  void AddVert(const VTX& vert) {
    ++m_added;
    if (m_added > 3 && (m_added & 1) == 0) {
      m_vec.reserve(m_vec.size() + 3);
      m_vec.push_back(m_vec.back());
      m_vec.push_back(m_vec[m_start]);
    }
    m_vec.push_back(vert);
  }

  template <class... _Args>
  void EmplaceVert(_Args&&... args) {
    ++m_added;
    if (m_added > 3 && (m_added & 1) == 0) {
      m_vec.reserve(m_vec.size() + 3);
      m_vec.push_back(m_vec.back());
      m_vec.push_back(m_vec[m_start]);
    }
    m_vec.emplace_back(std::forward<_Args>(args)...);
  }

  //  void Draw() const { CGraphics::DrawArray(m_start, m_vec.size() - m_start); }
};

#ifdef AURORA_GFX_DEBUG_GROUPS
struct ScopedDebugGroup {
  inline ScopedDebugGroup(const char* label) noexcept { push_debug_group(label); }
  inline ~ScopedDebugGroup() noexcept { pop_debug_group(); }
};
#define SCOPED_GRAPHICS_DEBUG_GROUP(name, ...)                                                                         \
  OPTICK_EVENT_DYNAMIC(name);                                                                                          \
  ScopedDebugGroup _GfxDbg_ { name }
#else
#define SCOPED_GRAPHICS_DEBUG_GROUP(name, ...) OPTICK_EVENT_DYNAMIC(name)
#endif

} // namespace metaforce
