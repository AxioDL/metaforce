#include "Runtime/Graphics/CGraphics.hpp"

#include "Runtime/CTimeProvider.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/Graphics/CGX.hpp"

#include <zeus/Math.hpp>

namespace metaforce {
CGraphics::CProjectionState CGraphics::g_Proj;
// CFogState CGraphics::g_Fog;
float CGraphics::g_ProjAspect = 1.f;
u32 CGraphics::g_NumBreakpointsWaiting = 0;
u32 CGraphics::g_FlippingState;
bool CGraphics::g_LastFrameUsedAbove = false;
bool CGraphics::g_InterruptLastFrameUsedAbove = false;
GX::LightMask CGraphics::g_LightActive{};
std::array<GXLightObj, GX::MaxLights> CGraphics::g_LightObjs;
std::array<ELightType, GX::MaxLights> CGraphics::g_LightTypes;
zeus::CTransform CGraphics::g_GXModelView;
zeus::CTransform CGraphics::g_GXModelViewInvXpose;
zeus::CTransform CGraphics::g_GXModelMatrix = zeus::CTransform();
zeus::CTransform CGraphics::g_ViewMatrix;
zeus::CVector3f CGraphics::g_ViewPoint;
zeus::CTransform CGraphics::g_GXViewPointMatrix;
zeus::CTransform CGraphics::g_CameraMatrix;
SClipScreenRect CGraphics::g_CroppedViewport;
bool CGraphics::g_IsGXModelMatrixIdentity = true;
zeus::CColor CGraphics::g_ClearColor = zeus::skClear;
float CGraphics::g_ClearDepthValue = 1.f;
bool CGraphics::g_IsBeginSceneClearFb = true;

SViewport CGraphics::g_Viewport = {
    0, 0, 640, 480, 640 / 2.f, 480 / 2.f, 0.0f,
};
u32 CGraphics::g_FrameCounter = 0;
u32 CGraphics::g_Framerate = 0;
u32 CGraphics::g_FramesPast = 0;
frame_clock::time_point CGraphics::g_FrameStartTime = frame_clock::now();
ERglEnum CGraphics::g_depthFunc = ERglEnum::Never;
ERglCullMode CGraphics::g_cullMode = ERglCullMode::None;

const std::array<zeus::CMatrix3f, 6> CGraphics::skCubeBasisMats{{
    /* Right */
    {0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f},
    /* Left */
    {0.f, -1.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, -1.f},
    /* Up */
    {1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f},
    /* Down */
    {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f},
    /* Back */
    {1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, -1.f},
    /* Forward */
    {-1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f},
}};
// We don't actually store anything here
u8 CGraphics::sSpareTextureData[] = {0};

// Stream API
static u32 sStreamFlags;
static GXPrimitive sStreamPrimitive;
static u32 sVerticesCount;
static zeus::CColor sQueuedColor;
// Originally writes directly to GX FIFO
struct StreamVertex {
  zeus::CColor color;
  zeus::CVector2f texCoord;
  zeus::CVector3f normal;
  zeus::CVector3f vertex;
  constexpr StreamVertex(const zeus::CColor& color) : color(color) {}
  constexpr StreamVertex(const StreamVertex&) = default;
};
static std::vector<StreamVertex> sQueuedVertices;

void CGraphics::DisableAllLights() {
  g_LightActive.reset();
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, {});
}

void CGraphics::LoadLight(ERglLight light, const CLight& info) {
  const auto lightId = static_cast<GXLightID>(1 << light);

  auto& obj = g_LightObjs[light];
  zeus::CVector3f pos = info.GetPosition();
  zeus::CVector3f dir = info.GetDirection();
  const auto type = info.GetType();
  if (type == ELightType::Directional) {
    dir = -(g_CameraMatrix.buildMatrix3f() * dir);
    GXInitLightPos(&obj, dir.x() * 1048576.f, dir.y() * 1048576.f, dir.z() * 1048576.f);
    GXInitLightAttn(&obj, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
  } else if (type == ELightType::Spot) {
    pos = g_CameraMatrix * pos;
    GXInitLightPos(&obj, pos.x(), pos.y(), pos.z());
    dir = g_CameraMatrix.buildMatrix3f() * dir;
    GXInitLightDir(&obj, dir.x(), dir.y(), dir.z());
    GXInitLightAttn(&obj, 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    GXInitLightSpot(&obj, info.GetSpotCutoff(), GX_SP_COS2);
  } else if (type == ELightType::Custom) {
    pos = g_CameraMatrix * pos;
    GXInitLightPos(&obj, pos.x(), pos.y(), pos.z());
    dir = g_CameraMatrix.buildMatrix3f() * dir;
    GXInitLightDir(&obj, dir.x(), dir.y(), dir.z());
    GXInitLightAttn(&obj, info.GetAngleAttenuationConstant(), info.GetAngleAttenuationLinear(),
                    info.GetAngleAttenuationQuadratic(), info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
  } else if (type == ELightType::LocalAmbient || type == ELightType::Point) {
    pos = g_CameraMatrix * pos;
    GXInitLightPos(&obj, pos.x(), pos.y(), pos.z());
    GXInitLightAttn(&obj, 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
  }

  g_LightTypes[light] = type;
  zeus::CColor col(info.GetColor().r(), info.GetColor().g(), info.GetColor().b());
  GXInitLightColor(&obj, to_gx_color(col));
  GXLoadLightObjImm(&obj, lightId);
}

void CGraphics::EnableLight(ERglLight light) {
  CGX::SetNumChans(1);
  if (!g_LightActive.test(light)) {
    g_LightActive.set(light);
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, g_LightActive);
  }
}

void CGraphics::SetLightState(GX::LightMask lightState) {
  g_LightActive = lightState;
  const bool hasLights = lightState.any();
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, hasLights, GX_SRC_REG,
                   sStreamFlags & 2 /* fHasColor */ ? GX_SRC_VTX : GX_SRC_REG, lightState,
                   hasLights ? GX_DF_CLAMP : GX_DF_NONE, hasLights ? GX_AF_SPOT : GX_AF_NONE);
}

void CGraphics::SetAmbientColor(const zeus::CColor& col) {
  CGX::SetChanAmbColor(CGX::EChannelId::Channel0, col);
  CGX::SetChanAmbColor(CGX::EChannelId::Channel1, col);
}

void CGraphics::SetFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) {
  CGX::SetFog(GXFogType(mode), startz, endz, g_Proj.x14_near, g_Proj.x18_far, to_gx_color(color));
}

void CGraphics::SetDepthWriteMode(bool compare_enable, ERglEnum comp, bool update_enable) {
  g_depthFunc = comp;
  CGX::SetZMode(compare_enable, GXCompare(comp), update_enable);
}

void CGraphics::SetBlendMode(ERglBlendMode mode, ERglBlendFactor src, ERglBlendFactor dst, ERglLogicOp op) {
  CGX::SetBlendMode(GXBlendMode(mode), GXBlendFactor(src), GXBlendFactor(dst), GXLogicOp(op));
}

void CGraphics::SetCullMode(ERglCullMode mode) {
  g_cullMode = mode;
  GXSetCullMode(GXCullMode(mode));
}

void CGraphics::BeginScene() {
  // ClearBackAndDepthBuffers();
}

void CGraphics::EndScene() {
  CGX::SetZMode(true, GX_LEQUAL, true);

  /* Spinwait until g_NumBreakpointsWaiting is 0 */
  /* ++g_NumBreakpointsWaiting; */
  /* GXCopyDisp to g_CurrenFrameBuf with clear enabled */
  /* Register next breakpoint with GP FIFO */

  /* Yup, GX effectively had fences long before D3D12 and Vulkan
   * (same functionality implemented in boo's execute method) */

  /* This usually comes from VI register during interrupt;
   * we don't care in the era of progressive-scan dominance,
   * so simulate field-flipping with XOR instead */
  g_InterruptLastFrameUsedAbove ^= 1;
  g_LastFrameUsedAbove = g_InterruptLastFrameUsedAbove;

  ++g_FrameCounter;

  UpdateFPSCounter();
}

void CGraphics::Render2D(CTexture& tex, u32 x, u32 y, u32 w, u32 h, const zeus::CColor& col) {
  const auto oldProj = g_Proj;
  const auto oldCull = g_cullMode;
  const auto oldLights = g_LightActive;
  SetOrtho(-g_Viewport.x10_halfWidth, g_Viewport.x10_halfWidth, g_Viewport.x14_halfHeight, -g_Viewport.x14_halfHeight,
           -1.f, -10.f);
  GXLoadPosMtxImm(&zeus::skIdentityMatrix4f, GX_PNMTX0);
  DisableAllLights();
  SetCullMode(ERglCullMode::None);
  tex.Load(GX_TEXMAP0, EClampMode::Repeat);

  //  float hPad, vPad;
  //  if (CGraphics::GetViewportAspect() >= 1.78f) {
  //    hPad = 1.78f / CGraphics::GetViewportAspect();
  //    vPad = 1.78f / 1.33f;
  //  } else {
  //    hPad = 1.f;
  //    vPad = CGraphics::GetViewportAspect() / 1.33f;
  //  }
  // TODO make this right
  float scaledX = static_cast<float>(x) / 640.f * static_cast<float>(g_Viewport.x8_width);
  float scaledY = static_cast<float>(y) / 448.f * static_cast<float>(g_Viewport.xc_height);
  float scaledW = static_cast<float>(w) / 640.f * static_cast<float>(g_Viewport.x8_width);
  float scaledH = static_cast<float>(h) / 448.f * static_cast<float>(g_Viewport.xc_height);

  float x1 = scaledX - g_Viewport.x10_halfWidth;
  float y1 = scaledY - g_Viewport.x14_halfHeight;
  float x2 = x1 + scaledW;
  float y2 = y1 + scaledH;
  StreamBegin(GX_TRIANGLESTRIP);
  StreamColor(col);
  StreamTexcoord(0.f, 0.f);
  StreamVertex(x1, y1, 1.f);
  StreamTexcoord(1.f, 0.f);
  StreamVertex(x2, y1, 1.f);
  StreamTexcoord(0.f, 1.f);
  StreamVertex(x1, y2, 1.f);
  StreamTexcoord(1.f, 1.f);
  StreamVertex(x2, y2, 1.f);
  StreamEnd();

  SetLightState(g_LightActive);
  g_Proj = oldProj;
  FlushProjection();
  SetModelMatrix({});
  SetCullMode(oldCull);
}

bool CGraphics::BeginRender2D(const CTexture& tex) { return false; }

void CGraphics::DoRender2D(const CTexture& tex, s32 x, s32 y, s32 w1, s32 w2, s32 w3, s32 w4, s32 w5,
                           const zeus::CColor& col) {}

void CGraphics::EndRender2D(bool v) {}

void CGraphics::SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1) {
  CGX::SetAlphaCompare(static_cast<GXCompare>(comp0), ref0, static_cast<GXAlphaOp>(op), static_cast<GXCompare>(comp1),
                       ref1);
}

void CGraphics::SetViewPointMatrix(const zeus::CTransform& xf) {
  g_ViewMatrix = xf;
  g_ViewPoint = xf.origin;
  zeus::CMatrix3f tmp(xf.basis[0], xf.basis[2], -xf.basis[1]);
  g_GXViewPointMatrix = zeus::CTransform(tmp.transposed());
  SetViewMatrix();
}

void CGraphics::SetViewMatrix() {
  g_CameraMatrix = g_GXViewPointMatrix * zeus::CTransform::Translate(-g_ViewPoint);
  if (g_IsGXModelMatrixIdentity)
    g_GXModelView = g_CameraMatrix;
  else
    g_GXModelView = g_CameraMatrix * g_GXModelMatrix;
  /* Load position matrix */
  GXLoadPosMtxImm(&g_GXModelView, GX_PNMTX0);
  /* Inverse-transpose */
  g_GXModelViewInvXpose = g_GXModelView.inverse();
  g_GXModelViewInvXpose.basis.transpose();
  /* Load normal matrix */
  GXLoadNrmMtxImm(&g_GXModelViewInvXpose, GX_PNMTX0);
}

void CGraphics::SetModelMatrix(const zeus::CTransform& xf) {
  g_IsGXModelMatrixIdentity = false;
  g_GXModelMatrix = xf;
  SetViewMatrix();
}

zeus::CMatrix4f CGraphics::CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar) {
  CProjectionState st;
  float tfov = std::tan(zeus::degToRad(fovy * 0.5f));
  st.x14_near = znear;
  st.x18_far = zfar;
  st.xc_top = znear * tfov;
  st.x10_bottom = -st.xc_top;
  st.x8_right = aspect * znear * tfov;
  st.x4_left = -st.x8_right;

  float rml = st.x8_right - st.x4_left;
  float rpl = st.x8_right + st.x4_left;
  float tmb = st.xc_top - st.x10_bottom;
  float tpb = st.xc_top + st.x10_bottom;
  float fpn = st.x18_far + st.x14_near;
  float fmn = st.x18_far - st.x14_near;
  // clang-format off
  return {
      2.f * st.x14_near / rml, 0.f, rpl / rml, 0.f,
      0.f, 2.f * st.x14_near / tmb, tpb / tmb, 0.f,
      0.f, 0.f, -fpn / fmn, -2.f * st.x18_far * st.x14_near / fmn,
      0.f, 0.f, -1.f, 0.f,
  };
  // clang-format on
}

zeus::CMatrix4f CGraphics::GetPerspectiveProjectionMatrix() {
  if (g_Proj.x0_persp) {
    float rml = g_Proj.x8_right - g_Proj.x4_left;
    float rpl = g_Proj.x8_right + g_Proj.x4_left;
    float tmb = g_Proj.xc_top - g_Proj.x10_bottom;
    float tpb = g_Proj.xc_top + g_Proj.x10_bottom;
    float fpn = g_Proj.x18_far + g_Proj.x14_near;
    float fmn = g_Proj.x18_far - g_Proj.x14_near;
    // clang-format off
    return {
        2.f * g_Proj.x14_near / rml, 0.f, rpl / rml, 0.f, 0.f,
        2.f * g_Proj.x14_near / tmb, tpb / tmb, 0.f,
        0.f, 0.f, -g_Proj.x14_near / fmn, -(g_Proj.x18_far * g_Proj.x14_near) / fmn,
        0.f, 0.f, -1.f, 0.f,
    };
    // clang-format on
  } else {
    float rml = g_Proj.x8_right - g_Proj.x4_left;
    float rpl = g_Proj.x8_right + g_Proj.x4_left;
    float tmb = g_Proj.xc_top - g_Proj.x10_bottom;
    float tpb = g_Proj.xc_top + g_Proj.x10_bottom;
    float fmn = g_Proj.x18_far - g_Proj.x14_near;
    // clang-format off
    return {
        2.f / rml, 0.f, 0.f, -rpl / rml,
        0.f, 2.f / tmb, 0.f, -tpb / tmb,
        0.f, 0.f, -1.f / fmn, -g_Proj.x18_far / fmn,
        0.f, 0.f, 0.f, 1.f
    };
    // clang-format on
  }
}

const CGraphics::CProjectionState& CGraphics::GetProjectionState() { return g_Proj; }

void CGraphics::SetProjectionState(const CGraphics::CProjectionState& proj) {
  g_Proj = proj;
  FlushProjection();
}

void CGraphics::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  g_ProjAspect = aspect;

  float tfov = std::tan(zeus::degToRad(fovy * 0.5f));
  g_Proj.x0_persp = true;
  g_Proj.x14_near = znear;
  g_Proj.x18_far = zfar;
  g_Proj.xc_top = znear * tfov;
  g_Proj.x10_bottom = -g_Proj.xc_top;
  g_Proj.x8_right = aspect * znear * tfov;
  g_Proj.x4_left = -g_Proj.x8_right;

  FlushProjection();
}

void CGraphics::SetOrtho(float left, float right, float top, float bottom, float znear, float zfar) {
  g_Proj.x0_persp = false;
  g_Proj.x4_left = left;
  g_Proj.x8_right = right;
  g_Proj.xc_top = top;
  g_Proj.x10_bottom = bottom;
  g_Proj.x14_near = znear;
  g_Proj.x18_far = zfar;

  FlushProjection();
}

void CGraphics::FlushProjection() {
  const auto mtx = GetPerspectiveProjectionMatrix();
  if (g_Proj.x0_persp) {
    // Convert and load persp
    GXSetProjection(&mtx, GX_PERSPECTIVE);
  } else {
    // Convert and load ortho
    GXSetProjection(&mtx, GX_ORTHOGRAPHIC);
  }
}

zeus::CVector2i CGraphics::ProjectPoint(const zeus::CVector3f& point) {
  zeus::CVector3f projPt = GetPerspectiveProjectionMatrix().multiplyOneOverW(point);
  return {int(projPt.x() * g_Viewport.x10_halfWidth) + int(g_Viewport.x10_halfWidth),
          int(g_Viewport.xc_height) - (int(projPt.y() * g_Viewport.x14_halfHeight) + int(g_Viewport.x14_halfHeight))};
}

SClipScreenRect CGraphics::ClipScreenRectFromMS(const zeus::CVector3f& p1, const zeus::CVector3f& p2) {
  zeus::CVector3f xf1 = g_GXModelView * p1;
  zeus::CVector3f xf2 = g_GXModelView * p2;
  return ClipScreenRectFromVS(xf1, xf2);
}

SClipScreenRect CGraphics::ClipScreenRectFromVS(const zeus::CVector3f& p1, const zeus::CVector3f& p2) {
  if (p1.x() == 0.f && p1.y() == 0.f && p1.z() == 0.f)
    return {};
  if (p2.x() == 0.f && p2.y() == 0.f && p2.z() == 0.f)
    return {};

  if (-p1.z() < GetProjectionState().x14_near || -p2.z() < GetProjectionState().x14_near)
    return {};
  if (-p1.z() > GetProjectionState().x18_far || -p2.z() > GetProjectionState().x18_far)
    return {};

  zeus::CVector2i sp1 = ProjectPoint(p1);
  zeus::CVector2i sp2 = ProjectPoint(p2);
  int minX = std::min(sp2.x, sp1.x);
  int minX2 = minX & 0xfffffffe;
  int minY = std::min(sp2.y, sp1.y);
  int minY2 = minY & 0xfffffffe;

  if (minX2 >= g_Viewport.x8_width)
    return {};

  int maxX = abs(sp1.x - sp2.x) + minX;
  int maxX2 = (maxX + 2) & 0xfffffffe;
  if (maxX2 <= 0 /* ViewportX origin */)
    return {};

  // int finalMinX = std::max(minX, 0 /* ViewportX origin */);
  // int finalMaxX = std::min(maxX, int(g_Viewport.x8_width));

  if (minY2 >= g_Viewport.xc_height)
    return {};

  int maxY = abs(sp1.y - sp2.y) + minY;
  int maxY2 = (maxY + 2) & 0xfffffffe;
  if (maxY2 <= 0 /* ViewportY origin */)
    return {};

  // int finalMinY = std::max(minY, 0 /* ViewportY origin */);
  // int finalMaxY = std::min(maxY, int(g_Viewport.xc_height));

  int width = maxX2 - minX2;
  int height = maxY2 - minY2;
  return {true,
          minX2,
          minY2,
          width,
          height,
          width,
          minX2 / float(g_Viewport.x8_width),
          maxX2 / float(g_Viewport.x8_width),
          1.f - maxY2 / float(g_Viewport.xc_height),
          1.f - minY2 / float(g_Viewport.xc_height)};
}

void CGraphics::SetViewportResolution(const zeus::CVector2i& res) {
  g_Viewport.x8_width = res.x;
  g_Viewport.xc_height = res.y;
  g_CroppedViewport = SClipScreenRect();
  g_CroppedViewport.xc_width = res.x;
  g_CroppedViewport.x10_height = res.y;
  g_Viewport.x10_halfWidth = res.x / 2.f;
  g_Viewport.x14_halfHeight = res.y / 2.f;
  g_Viewport.aspect = res.x / float(res.y);
  if (g_GuiSys)
    g_GuiSys->OnViewportResize();
}

static zeus::CRectangle CachedVP;
zeus::CVector2f CGraphics::g_CachedDepthRange = {0.f, 1.f};

void CGraphics::SetViewport(int leftOff, int bottomOff, int width, int height) {
  CachedVP.position[0] = leftOff;
  CachedVP.position[1] = bottomOff;
  CachedVP.size[0] = width;
  CachedVP.size[1] = height;
  GXSetViewport(CachedVP.position[0], CachedVP.position[1], CachedVP.size[0], CachedVP.size[1], g_CachedDepthRange[0],
                g_CachedDepthRange[1]);
}

void CGraphics::SetScissor(int leftOff, int bottomOff, int width, int height) {
  GXSetScissor(leftOff, bottomOff, width, height);
}

void CGraphics::SetDepthRange(float znear, float zfar) {
  g_CachedDepthRange[0] = znear;
  g_CachedDepthRange[1] = zfar;
  GXSetViewport(CachedVP.position[0], CachedVP.position[1], CachedVP.size[0], CachedVP.size[1], g_CachedDepthRange[0],
                g_CachedDepthRange[1]);
}

CTimeProvider* CGraphics::g_ExternalTimeProvider = nullptr;
float CGraphics::g_DefaultSeconds = 0.f;
u32 CGraphics::g_RenderTimings = 0;

float CGraphics::GetSecondsMod900() {
  if (!g_ExternalTimeProvider)
    return g_DefaultSeconds;
  return g_ExternalTimeProvider->x0_currentTime;
}

void CGraphics::TickRenderTimings() {
  OPTICK_EVENT();
  g_RenderTimings = (g_RenderTimings + 1) % u32(900 * 60);
  g_DefaultSeconds = g_RenderTimings / 60.f;
}

static constexpr u64 FPS_REFRESH_RATE = 1000;
void CGraphics::UpdateFPSCounter() {
  ++g_FramesPast;

  std::chrono::duration<double, std::milli> timeElapsed = frame_clock::now() - g_FrameStartTime;
  if (timeElapsed.count() > FPS_REFRESH_RATE) {
    g_Framerate = g_FramesPast;
    g_FrameStartTime = frame_clock::now();
    g_FramesPast = 0;
  }
}

static bool g_UseVideoFilter = false;
void CGraphics::SetUseVideoFilter(bool filter) {
  g_UseVideoFilter = filter;
  // GXSetCopyFilter(CGraphics::mRenderModeObj.aa, CGraphics::mRenderModeObj.sample_pattern, filter,
  //                 CGraphics::mRenderModeObj.vfilter);
}

void CGraphics::SetClearColor(const zeus::CColor& color) {
  g_ClearColor = color;
  GXSetCopyClear(to_gx_color(color), g_ClearDepthValue);
}

void CGraphics::SetCopyClear(const zeus::CColor& color, float depth) {
  g_ClearColor = color;
  g_ClearDepthValue = depth; // 1.6777215E7 * depth; Metroid Prime needed this to convert float [0,1] depth into 24 bit
                             // range, we no longer have this requirement
  GXSetCopyClear(to_gx_color(g_ClearColor), g_ClearDepthValue);
}

void CGraphics::SetIsBeginSceneClearFb(bool clear) { g_IsBeginSceneClearFb = clear; }

void CGraphics::SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass) {
  CTevCombiners::SetupPass(stage, pass);
}

void CGraphics::StreamBegin(GXPrimitive primitive) {
  // Originally ResetVertexDataStream(true);
  sQueuedVertices.clear();
  sQueuedVertices.emplace_back(sQueuedColor);
  sVerticesCount = 0;
  // End
  sStreamFlags = 2;
  sStreamPrimitive = primitive;
}

void CGraphics::StreamNormal(const zeus::CVector3f& nrm) {
  sQueuedVertices.back().normal = nrm;
  sStreamFlags |= 1;
}

void CGraphics::StreamColor(const zeus::CColor& color) {
  if (sStreamFlags) {
    sQueuedVertices.back().color = color;
  } else {
    sQueuedColor = color;
  }
  sStreamFlags |= 2;
}

void CGraphics::StreamTexcoord(const zeus::CVector2f& uv) {
  sQueuedVertices.back().texCoord = uv;
  sStreamFlags |= 4;
}

void CGraphics::StreamVertex(const zeus::CVector3f& pos) {
  sQueuedVertices.back().vertex = pos;
  UpdateVertexDataStream();
}

void CGraphics::StreamEnd() {
  if (sVerticesCount != 0) {
    FlushStream();
  }
  sStreamFlags = {};
}

void CGraphics::UpdateVertexDataStream() {
  ++sVerticesCount;
  if (sVerticesCount == 240) {
    FlushStream();
    ResetVertexDataStream(false);
  } else {
    sQueuedVertices.emplace_back(sQueuedVertices.back());
  }
}

void CGraphics::FlushStream() {
  std::array<GXVtxDescList, 5> vtxDescList{};
  size_t idx = 0;
  vtxDescList[idx++] = {GX_VA_POS, GX_DIRECT};
  if (sStreamFlags & 1) {
    vtxDescList[idx++] = {GX_VA_NRM, GX_DIRECT};
  }
  if (sStreamFlags & 2) {
    vtxDescList[idx++] = {GX_VA_CLR0, GX_DIRECT};
  }
  if (sStreamFlags & 4) {
    vtxDescList[idx++] = {GX_VA_TEX0, GX_DIRECT};
  }
  vtxDescList[idx] = {GX_VA_NULL, GX_NONE};
  CGX::SetVtxDescv(vtxDescList.data());
  SetTevStates(sStreamFlags);
  FullRender();
}

void CGraphics::FullRender() {
  CGX::Begin(sStreamPrimitive, GX_VTXFMT0, sVerticesCount);
  for (size_t i = 0; i < sVerticesCount; ++i) {
    const auto& item = sQueuedVertices[i];
    GXPosition3f32(item.vertex);
    if (sStreamFlags & 1) {
      GXNormal3f32(item.normal);
    }
    if (sStreamFlags & 2) {
      GXColor4f32(item.color);
    }
    if (sStreamFlags & 4) {
      GXTexCoord2f32(item.texCoord);
    }
  }
  CGX::End();
}

void CGraphics::ResetVertexDataStream(bool end) {
  if (end) {
    sQueuedVertices.clear();
  } else {
    const auto lastVertex = sQueuedVertices.back();
    sQueuedVertices.clear();
    sQueuedVertices.emplace_back(lastVertex);
  }
  sVerticesCount = 0;
  if (!end) {
    // TODO something with triangle fans
  }
}

void CGraphics::DrawPrimitive(GXPrimitive primitive, const zeus::CVector3f* pos, const zeus::CVector3f& normal,
                              const zeus::CColor& col, s32 numVerts) {
  StreamBegin(primitive);
  StreamColor(col);
  StreamNormal(normal);
  for (u32 i = 0; i < numVerts; ++i) {
    StreamVertex(pos[i]);
  }
  StreamEnd();
}

void CGraphics::SetTevStates(u32 flags) noexcept {
  if (flags & 4 /* fHasTexture */) {
    CGX::SetNumTexGens(1); // sTextureUsed & 3?
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
  } else {
    CGX::SetNumTexGens(0);
    CGX::SetNumTevStages(1);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  }
  CGX::SetNumChans(1);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  const bool hasLights = g_LightActive.any();
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, hasLights, GX_SRC_REG,
                   flags & 2 /* fHasColor */ ? GX_SRC_VTX : GX_SRC_REG, g_LightActive,
                   hasLights ? GX_DF_CLAMP : GX_DF_NONE, hasLights ? GX_AF_SPOT : GX_AF_NONE);
  CGX::FlushState(); // normally would be handled in FullRender TODO
}

void CGraphics::Startup() {
  // Setup GXFifo
  CGX::ResetGXStates();
  InitGraphicsVariables();
  // ConfigureFrameBuffer(...);
  InitGraphicsDefaults();
  // GXInitTexCacheRegion
  // GXSetTexRegionCallback
}

void CGraphics::InitGraphicsVariables() {
  g_LightTypes.fill(ELightType::Directional);
  g_LightActive = {};
  SetDepthWriteMode(false, g_depthFunc, false);
  SetCullMode(ERglCullMode::None);
  SetAmbientColor(zeus::CColor{0.2f, 1.f});
  g_IsGXModelMatrixIdentity = false;
  // SetIdentityViewPointMatrix();
  // SetIdentityModelMatrix();
  SetViewport(0, 0, g_Viewport.x8_width, g_Viewport.xc_height);
  SetPerspective(60.f, g_Viewport.x8_width / g_Viewport.xc_height, g_Proj.x14_near, g_Proj.x18_far);
  SetCopyClear(g_ClearColor, 1.f);
  CGX::SetChanMatColor(CGX::EChannelId::Channel0, zeus::skWhite);
  // g_RenderState.ResetFlushAll();
}

void CGraphics::InitGraphicsDefaults() {
  SetDepthRange(0.f, 1.f);
  g_IsGXModelMatrixIdentity = false;
  SetModelMatrix(g_GXModelMatrix);
  SetViewPointMatrix(g_GXModelView);
  SetDepthWriteMode(false, g_depthFunc, false);
  SetCullMode(g_cullMode);
  SetViewport(g_Viewport.x0_left, g_Viewport.x4_top, g_Viewport.x8_width, g_Viewport.xc_height);
  FlushProjection();
  CTevCombiners::Init();
  DisableAllLights();
  SetDefaultVtxAttrFmt();
}

void CGraphics::SetDefaultVtxAttrFmt() {
  // Unneeded, all attributes are expected to be full floats
  // Left here for reference

  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);
  for (GXAttr attr = GX_VA_TEX1; attr <= GX_VA_TEX7; attr = GXAttr(attr + 1)) {
    GXSetVtxAttrFmt(GX_VTXFMT0, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, attr, GX_TEX_ST, GX_F32, 0);
  }
}

void CGraphics::ResetGfxStates() noexcept {
  // sRenderState.x0_ = 0;
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXTexFmt format, void* data, GXTexMapID id) {
  void* ptr = static_cast<void*>(sSpareTextureData);
  if (data != nullptr) {
    ptr = data;
  }
  GXTexObj obj;
  GXInitTexObj(&obj, ptr, width, height, format, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, false, false, GX_ANISO_1);
  GXLoadTexObj(&obj, id);
  // CTexture::InvalidateTexmap(id);
  if (id == GX_TEXMAP7) {
    // GXInvalidateTexRegion(...);
  }
#ifdef AURORA
  GXDestroyTexObj(&obj);
#endif
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXCITexFmt format, GXTlut tlut, void* data,
                                        GXTexMapID id) {
  void* ptr = static_cast<void*>(sSpareTextureData);
  if (data != nullptr) {
    ptr = data;
  }
  GXTexObj obj;
  GXInitTexObjCI(&obj, ptr, width, height, format, GX_CLAMP, GX_CLAMP, false, tlut);
  GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, false, false, GX_ANISO_1);
  GXLoadTexObj(&obj, id);
  // CTexture::InvalidateTexmap(id);
  if (id == GX_TEXMAP7) {
    // GXInvalidateTexRegion(...);
  }
#ifdef AURORA
  GXDestroyTexObj(&obj);
#endif
}
} // namespace metaforce
