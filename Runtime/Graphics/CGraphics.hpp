#pragma once

#include "Runtime/ConsoleVariables/CVar.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/CTevCombiners.hpp"
#include "Runtime/Graphics/GX.hpp"
#include "Runtime/RetroTypes.hpp"

#include <vector>

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector2i.hpp>

#include <aurora/aurora.h>
//#include <optick.h>
#include <dolphin/mtx.h>

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

enum class ERglPrimitive : std::underlying_type_t<GXPrimitive> {
  Quads = GX_QUADS,
  Triangles = GX_TRIANGLES,
  TriangleStrip = GX_TRIANGLESTRIP,
  TriangleFan = GX_TRIANGLEFAN,
  Lines = GX_LINES,
  LineStrip = GX_LINESTRIP,
  Points = GX_POINTS,
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

struct CViewport {
  int mLeft;
  int mTop;
  int mWidth;
  int mHeight;
  float mHalfWidth;
  float mHalfHeight;
};

// TODO
typedef struct {
  float x;
  float y;
} Vec2, *Vec2Ptr;

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

enum class ETexelFormat;

class CGraphics {
public:
  using CVector3f = zeus::CVector3f;
  using CTransform4f = zeus::CTransform;
  using CColor = zeus::CColor;
  using uchar = unsigned char;
  using uint = unsigned int;

  class CRenderState {
  public:
    CRenderState();

    void Flush();
    void ResetFlushAll();
    int SetVtxState(const float* pos, const float* nrm, const uint* clr);

    // In map this takes two args, but x4 is unused?
    void Set(int v0) { x0_ = v0; }

  private:
    int x0_;
    int x4_;
  };

  class CProjectionState {
  public:
    CProjectionState(bool persp, float left, float right, float top, float bottom, float near, float far)
    : x0_persp(persp), x4_left(left), x8_right(right), xc_top(top), x10_bottom(bottom), x14_near(near), x18_far(far) {}

    bool IsPerspective() const { return x0_persp; }
    float GetLeft() const { return x4_left; }
    float GetRight() const { return x8_right; }
    float GetTop() const { return xc_top; }
    float GetBottom() const { return x10_bottom; }
    float GetNear() const { return x14_near; }
    float GetFar() const { return x18_far; }

  private:
    bool x0_persp;
    float x4_left;
    float x8_right;
    // TODO: I think top/bottom are flipped
    float xc_top;
    float x10_bottom;
    float x14_near;
    float x18_far;
  };

  class CClippedScreenRect {
  public:
    CClippedScreenRect() : x0_valid(false) {}
    CClippedScreenRect(int x, int y, int width, int height, int texWidth, float minU, float maxU, float minV,
                       float maxV)
    : x0_valid(true)
    , x4_x(x)
    , x8_y(y)
    , xc_width(width)
    , x10_height(height)
    , x14_texWidth(texWidth)
    , x18_minU(minU)
    , x1c_maxU(maxU)
    , x20_minV(minV)
    , x24_maxV(maxV) {}

    bool IsValid() const { return x0_valid; }
    int GetX() const { return x4_x; }
    int GetY() const { return x8_y; }
    int GetWidth() const { return xc_width; }
    int GetHeight() const { return x10_height; }
    int GetTexWidth() const { return x14_texWidth; }
    float GetMinU() const { return x18_minU; }
    float GetMaxU() const { return x1c_maxU; }
    float GetMinV() const { return x20_minV; }
    float GetMaxV() const { return x24_maxV; }

  private:
    bool x0_valid;
    int x4_x;
    int x8_y;
    int xc_width;
    int x10_height;
    int x14_texWidth;
    float x18_minU;
    float x1c_maxU;
    float x20_minV;
    float x24_maxV;
  };

  static CRenderState sRenderState;
  static VecPtr vtxBuffer;
  static VecPtr nrmBuffer;
  static Vec2Ptr txtBuffer0;
  static Vec2Ptr txtBuffer1;
  static uint* clrBuffer;
  static bool mJustReset;
  static ERglCullMode mCullMode;
  static int mNumLightsActive;
  static float mDepthNear;
  static VecPtr mpVtxBuffer;
  static VecPtr mpNrmBuffer;
  static Vec2Ptr mpTxtBuffer0;
  static Vec2Ptr mpTxtBuffer1;
  static uint* mpClrBuffer;
  static int mNumPrimitives;
  static int mFrameCounter;
  static float mFramesPerSecond;
  static float mLastFramesPerSecond;
  static int mNumBreakpointsWaiting;
  static int mFlippingState;
  static bool mLastFrameUsedAbove;
  static bool mInterruptLastFrameUsedAbove;
  static GX::LightMask mLightActive;
  static GX::LightMask mLightsWereOn;
  static void* mpFrameBuf1;
  static void* mpFrameBuf2;
  static void* mpCurrenFrameBuf;
  static int mSpareBufferSize;
  static void* mpSpareBuffer;
  static int mSpareBufferTexCacheSize;
  // static GXTexRegionCallback mGXDefaultTexRegionCallback;
  static void* mpFifo;
  static GXFifoObj* mpFifoObj;
  static uint mRenderTimings;
  static float mSecondsMod900;
  static CTimeProvider* mpExternalTimeProvider;
  static int mScreenStretch;
  static int mScreenPositionX;
  static int mScreenPositionY;

  static CVector3f kDefaultPositionVector;
  static CVector3f kDefaultDirectionVector;
  static CProjectionState mProj;
  static CTransform4f mViewMatrix;
  static CTransform4f mModelMatrix;
  static CColor mClearColor;
  static CVector3f mViewPoint;
  static CViewport mViewport;
  static ELightType mLightTypes[8];
  static GXLightObj mLightObj[8];
  // static GXTexRegion mTexRegions[GX_MAX_TEXMAP];
  // static GXTexRegion mTexRegionsCI[GX_MAX_TEXMAP / 2];
  static GXRenderModeObj mRenderModeObj;
  static Mtx mGXViewPointMatrix;
  static Mtx mGXModelMatrix;
  static Mtx mGxModelView;
  static Mtx mCameraMtx;

  static bool mIsBeginSceneClearFb;
  static ERglEnum mDepthFunc;
  static ERglPrimitive mCurrentPrimitive;
  static float mDepthFar;
  static u32 mClearDepthValue; // = GX_MAX_Z24
  static bool mIsGXModelMatrixIdentity;
  static bool mFirstFrame;
  static GXBool mUseVideoFilter;
  static float mBrightness;

  static const GXTexMapID kSpareBufferTexMapID;

  static bool Startup();
  static void InitGraphicsVariables();
  static void InitGraphicsDefaults();
  static void ConfigureFrameBuffer();
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
  static void Render2D(CTexture& tex, int x, int y, int w, int h, const zeus::CColor& col, bool scale);
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
  static CClippedScreenRect ClipScreenRectFromVS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt);
  static CClippedScreenRect ClipScreenRectFromMS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt);

  static void SetViewportResolution(const zeus::CVector2i& res);
  static void SetViewport(int leftOff, int bottomOff, int width, int height);
  static void SetScissor(int leftOff, int bottomOff, int width, int height);
  static void SetDepthRange(float near, float far);
  static void SetIdentityViewPointMatrix();
  static void SetIdentityModelMatrix();
  static void ClearBackAndDepthBuffers();

  static void SetExternalTimeProvider(CTimeProvider* provider) { mpExternalTimeProvider = provider; }
  static float GetSecondsMod900();
  static void TickRenderTimings();
  static int GetFrameCounter() { return mFrameCounter; }
  static float GetFPS() { return mFramesPerSecond; }
  static void SetUseVideoFilter(bool);
  static void SetClearColor(const zeus::CColor& color);
  static void SetCopyClear(const zeus::CColor& color, float depth);
  static void SetIsBeginSceneClearFb(bool clear);
  static int GetViewportLeft() { return mViewport.mLeft; }
  static int GetViewportTop() { return mViewport.mTop; }
  static int GetViewportWidth() { return mViewport.mWidth; }
  static int GetViewportHeight() { return mViewport.mHeight; }
  static float GetViewportHalfWidth() { return mViewport.mHalfWidth; }
  static float GetViewportHalfHeight() { return mViewport.mHalfHeight; }
  static float GetViewportAspect() {
    return static_cast<float>(mViewport.mWidth) / static_cast<float>(mViewport.mHeight);
  }
  static const CVector3f& GetViewPoint() { return mViewPoint; }
  static const CTransform4f& GetViewMatrix() { return mViewMatrix; }
  static const CTransform4f& GetModelMatrix() { return mModelMatrix; }
  static GX::LightMask GetLightMask() { return mLightActive; }

  static void LoadDolphinSpareTexture(int width, int height, GXTexFmt format, void* data, GXTexMapID id);
  static void LoadDolphinSpareTexture(int width, int height, GXCITexFmt format, GXTlut tlut, void* data, GXTexMapID id);

  static void ResetGfxStates() noexcept;
  static void SetTevStates(u32 flags) noexcept;
  static void SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass);
  static void StreamBegin(ERglPrimitive primitive);
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
  static void DrawPrimitive(ERglPrimitive primitive, const zeus::CVector3f* pos, const zeus::CVector3f& normal,
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
#define SCOPED_GRAPHICS_DEBUG_GROUP(name, ...) 
  //OPTICK_EVENT_DYNAMIC(name)
#endif

} // namespace metaforce
