#include "Graphics/CGraphics.hpp"
#include "Graphics/CLight.hpp"
#include "zeus/Math.hpp"
#include "CTimeProvider.hpp"

#undef near
#undef far

namespace urde
{

CGraphics::CProjectionState CGraphics::g_Proj;
float CGraphics::g_ProjAspect = 1.f;
u32 CGraphics::g_NumLightsActive = 0;
u32 CGraphics::g_NumBreakpointsWaiting = 0;
u32 CGraphics::g_FlippingState;
bool CGraphics::g_LastFrameUsedAbove = false;
bool CGraphics::g_InterruptLastFrameUsedAbove = false;
ERglLightBits CGraphics::g_LightActive = ERglLightBits::None;
ERglLightBits CGraphics::g_LightsWereOn = ERglLightBits::None;
zeus::CTransform CGraphics::g_GXModelView;
zeus::CTransform CGraphics::g_GXModelViewInvXpose;
zeus::CTransform CGraphics::g_GXModelMatrix;
zeus::CTransform CGraphics::g_ViewMatrix;
zeus::CVector3f CGraphics::g_ViewPoint;
zeus::CTransform CGraphics::g_GXViewPointMatrix;
zeus::CTransform CGraphics::g_CameraMatrix;
zeus::CVector2i CGraphics::g_ViewportResolution;
zeus::CVector2i CGraphics::g_ViewportResolutionHalf;
int CGraphics::g_ViewportSamples = 1;
bool CGraphics::g_IsGXModelMatrixIdentity;

void CGraphics::DisableAllLights()
{
    g_NumLightsActive = 0;
    g_LightActive = ERglLightBits::None;
    // TODO: turn lights off for real
}

void CGraphics::LoadLight(ERglLight light, const CLight& info)
{
    // TODO: load light for real
}

void CGraphics::EnableLight(ERglLight light)
{
    ERglLightBits lightBit = ERglLightBits(1 << int(light));
    if ((lightBit & g_LightActive) == ERglLightBits::None)
    {
        g_LightActive |= lightBit;
        ++g_NumLightsActive;
        // TODO: turn light on for real
    }
    g_LightsWereOn = g_LightActive;
}

void CGraphics::SetLightState(ERglLightBits lightState)
{
    // TODO: set state for real
    g_LightActive = lightState;
    g_NumLightsActive = zeus::PopCount(lightState);
}

void CGraphics::SetAmbientColor(const zeus::CColor& col)
{
    // TODO: set for real
}

void CGraphics::SetDepthWriteMode(bool test, ERglEnum comp, bool write)
{

}

void CGraphics::SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp)
{
}

void CGraphics::SetCullMode(ERglCullMode)
{
}

void CGraphics::EndScene()
{
    /* Spinwait until g_NumBreakpointsWaiting is 0 */
    /* ++g_NumBreakpointsWaiting; */
    /* GXCopyDisp to g_CurrenFrameBuf with clear enabled */
    /* Register next breakpoint with GP FIFO */

    /* Yup, GX had fences long before D3D12 and Vulkan
     * (same functionality implemented in boo's execute method) */

    /* This usually comes from VI register during interrupt;
     * we don't care in the era of progressive-scan dominance,
     * so simulate field-flipping with XOR instead */
    g_InterruptLastFrameUsedAbove ^= 1;
    g_LastFrameUsedAbove = g_InterruptLastFrameUsedAbove;
}

void CGraphics::SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1)
{
}

void CGraphics::SetViewPointMatrix(const zeus::CTransform& xf)
{
    g_ViewMatrix = xf;
    g_ViewPoint = xf.m_origin;
    zeus::CMatrix3f tmp(xf.m_basis[0], xf.m_basis[2], -xf.m_basis[1]);
    g_GXViewPointMatrix = zeus::CTransform(tmp.transposed());
    SetViewMatrix();
}

void CGraphics::SetViewMatrix()
{
    g_CameraMatrix = g_GXViewPointMatrix * zeus::CTransform::Translate(-g_ViewPoint);
    if (g_IsGXModelMatrixIdentity)
        g_GXModelView = g_CameraMatrix;
    else
        g_GXModelView = g_CameraMatrix * g_GXModelMatrix;
    /* Load position matrix */
    /* Inverse-transpose */
    g_GXModelViewInvXpose = g_GXModelView.inverse();
    g_GXModelViewInvXpose.m_origin.zeroOut();
    g_GXModelViewInvXpose.m_basis.transpose();
    /* Load normal matrix */
}

void CGraphics::SetModelMatrix(const zeus::CTransform& xf)
{
    g_IsGXModelMatrixIdentity = false;
    g_GXModelMatrix = xf;
    SetViewMatrix();
}

zeus::CMatrix4f CGraphics::CalculatePerspectiveMatrix(float fovy, float aspect,
                                                      float near, float far)
{
    CProjectionState st;
    float tfov = std::tan(zeus::degToRad(fovy * 0.5f));
    st.x14_near = near;
    st.x18_far = far;
    st.xc_top = near * tfov;
    st.x10_bottom = -st.xc_top;
    st.x8_right = aspect * near * tfov;
    st.x4_left = -st.x8_right;

    float rml = st.x8_right - st.x4_left;
    float rpl = st.x8_right + st.x4_left;
    float tmb = st.xc_top - st.x10_bottom;
    float tpb = st.xc_top + st.x10_bottom;
    float fmn = st.x18_far - st.x14_near;
    float fpn = st.x18_far + st.x14_near;
    return zeus::CMatrix4f(2.f * st.x14_near / rml, 0.f, rpl / rml, 0.f,
                           0.f, 2.f * st.x14_near / tmb, tpb / tmb, 0.f,
                           0.f, 0.f, -fpn / fmn, -2.f * st.x18_far * st.x14_near / fmn,
                           0.f, 0.f, -1.f, 0.f);
}

zeus::CMatrix4f CGraphics::GetPerspectiveProjectionMatrix()
{
    float rml = g_Proj.x8_right - g_Proj.x4_left;
    float rpl = g_Proj.x8_right + g_Proj.x4_left;
    float tmb = g_Proj.xc_top - g_Proj.x10_bottom;
    float tpb = g_Proj.xc_top + g_Proj.x10_bottom;
    float nmf = g_Proj.x14_near - g_Proj.x18_far;
    float fpn = g_Proj.x18_far + g_Proj.x14_near;
    return zeus::CMatrix4f(2.f * g_Proj.x14_near / rml, 0.f, rpl / rml, 0.f,
                           0.f, 2.f * g_Proj.x14_near / tmb, tpb / tmb, 0.f,
                           0.f, 0.f, fpn / nmf, 2.f * g_Proj.x18_far * g_Proj.x14_near / nmf,
                           0.f, 0.f, -1.f, 0.f);
}

const CGraphics::CProjectionState& CGraphics::GetProjectionState()
{
    return g_Proj;
}

void CGraphics::SetProjectionState(const CGraphics::CProjectionState& proj)
{
    g_Proj = proj;
    FlushProjection();
}

void CGraphics::SetPerspective(float fovy, float aspect, float near, float far)
{
    g_ProjAspect = aspect;

    float tfov = std::tan(zeus::degToRad(fovy * 0.5f));
    g_Proj.x0_persp = true;
    g_Proj.x14_near = near;
    g_Proj.x18_far = far;
    g_Proj.xc_top = near * tfov;
    g_Proj.x10_bottom = -g_Proj.xc_top;
    g_Proj.x8_right = aspect * near * tfov;
    g_Proj.x4_left = -g_Proj.x8_right;

    FlushProjection();
}

void CGraphics::SetOrtho(float left, float right,
                         float top, float bottom,
                         float znear, float zfar)
{
    g_Proj.x0_persp = false;
    g_Proj.x4_left = left;
    g_Proj.x8_right = right;
    g_Proj.xc_top = top;
    g_Proj.x10_bottom = bottom;
    g_Proj.x14_near = znear;
    g_Proj.x18_far = zfar;

    FlushProjection();
}

void CGraphics::FlushProjection()
{
    if (g_Proj.x0_persp)
    {
        // Convert and load persp
    }
    else
    {
        // Convert and load ortho
    }
}

zeus::CVector2i CGraphics::ProjectPoint(const zeus::CVector3f& point)
{
    zeus::CVector3f projPt = GetPerspectiveProjectionMatrix().multiplyOneOverW(point);
    return {int(projPt.x * g_ViewportResolutionHalf.x) + g_ViewportResolutionHalf.x,
            g_ViewportResolution.y - (int(projPt.y * g_ViewportResolutionHalf.y) + g_ViewportResolutionHalf.y)};
}

SClipScreenRect CGraphics::ClipScreenRectFromMS(const zeus::CVector3f& p1,
                                                const zeus::CVector3f& p2)
{
    zeus::CVector3f xf1 = g_GXModelView * p1;
    zeus::CVector3f xf2 = g_GXModelView * p2;
    return ClipScreenRectFromVS(xf1, xf2);
}

SClipScreenRect CGraphics::ClipScreenRectFromVS(const zeus::CVector3f& p1,
                                                const zeus::CVector3f& p2)
{
    if (p1.x == 0.f && p1.y == 0.f && p1.z == 0.f)
        return {};
    if (p2.x == 0.f && p2.y == 0.f && p2.z == 0.f)
        return {};

    if (p1.y < GetProjectionState().x14_near || p2.y < GetProjectionState().x14_near)
        return {};
    if (p1.y > GetProjectionState().x18_far || p2.y > GetProjectionState().x18_far)
        return {};

    zeus::CVector2i sp1 = ProjectPoint(p1);
    zeus::CVector2i sp2 = ProjectPoint(p2);
    int minX = std::min(sp2.x, sp1.x);
    int minX2 = minX & 0xfffffffe;
    int minY = std::min(sp2.y, sp1.y);
    int minY2 = minY & 0xfffffffe;


    if (minX2 >= g_ViewportResolution.x)
        return {};

    int maxX = abs(sp1.x - sp2.x) + minX;
    int maxX2 = (maxX + 2) & 0xfffffffe;
    if (maxX2 <= 0 /* ViewportX origin */)
        return {};

    int finalMinX = std::max(minX, 0 /* ViewportX origin */);
    int finalMaxX = std::min(maxX, g_ViewportResolution.x);


    if (minY2 >= g_ViewportResolution.y)
        return {};

    int maxY = abs(sp1.y - sp2.y) + minY;
    int maxY2 = (maxY + 2) & 0xfffffffe;
    if (maxY2 <= 0 /* ViewportY origin */)
        return {};

    int finalMinY = std::max(minY, 0 /* ViewportY origin */);
    int finalMaxY = std::min(maxY, g_ViewportResolution.y);

    int width = maxX2 - minX2;
    int height = maxY2 - minY2;
    return {true, minX2, minY2, width, height, width,
            minX2 / float(g_ViewportResolution.x), maxX2 / float(g_ViewportResolution.x),
            1.f - maxY2 / float(g_ViewportResolution.y), 1.f - minY2 / float(g_ViewportResolution.y)};

}

zeus::CVector3f CGraphics::ProjectModelPointToViewportSpace(const zeus::CVector3f& point)
{
    zeus::CVector3f pt = g_GXModelView * point;
    return GetPerspectiveProjectionMatrix().multiplyOneOverW(pt);
}

void CGraphics::SetViewportResolution(const zeus::CVector2i& res)
{
    g_ViewportResolution = res;
    g_ViewportResolutionHalf = {res.x / 2, res.y / 2};
}

CTimeProvider* CGraphics::g_ExternalTimeProvider = nullptr;
float CGraphics::g_DefaultSeconds;

float CGraphics::GetSecondsMod900()
{
    if (!g_ExternalTimeProvider)
        return g_DefaultSeconds;
    return g_ExternalTimeProvider->x0_currentTime;
}

boo::IGraphicsDataFactory* CGraphics::g_BooFactory = nullptr;
boo::IGraphicsCommandQueue* CGraphics::g_BooMainCommandQueue = nullptr;
boo::ITextureR* CGraphics::g_SpareTexture = nullptr;
hecl::Runtime::ShaderCacheManager* CGraphics::g_ShaderCacheMgr = nullptr;

}
