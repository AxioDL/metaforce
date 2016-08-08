#ifndef __URDE_CGRAPHICS_HPP__
#define __URDE_CGRAPHICS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CVector2i.hpp"
#include "zeus/CColor.hpp"

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/graphicsdev/IGraphicsCommandQueue.hpp"

#include "hecl/Runtime.hpp"

namespace urde
{
class CLight;
class CTimeProvider;

enum class ERglLight : u8
{
    Zero = 0,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven
};

enum class ERglLightBits : u8
{
    None  = 0,
    Zero  = 1,
    One   = 1 << 1,
    Two   = 1 << 2,
    Three = 1 << 3,
    Four  = 1 << 4,
    Five  = 1 << 5,
    Six   = 1 << 6,
    Seven = 1 << 7
};
ENABLE_BITWISE_ENUM(ERglLightBits)

enum class ERglEnum
{
    Never   = 0,
    Less    = 1,
    Equal   = 2,
    LEqual  = 3,
    Greater = 4,
    NEqual  = 5,
    GEqual  = 6,
    Always  = 7
};

enum class ERglBlendMode
{
    None     = 0,
    Blend    = 1,
    Logic    = 2,
    Subtract = 3
};

enum class ERglBlendFactor
{
    Zero        = 0,
    One         = 1,
    SrcColor    = 2,
    InvSrcColor = 3,
    SrcAlpha    = 4,
    InvSrcAlpha = 5,
    DstAlpha    = 6,
    InvDstAlpha = 7
};

enum class ERglLogicOp
{
    Clear   = 0,
    And     = 1,
    RevAnd  = 2,
    Copy    = 3,
    InvAnd  = 4,
    NoOp    = 5,
    Xor     = 6,
    Or      = 7,
    Nor     = 8,
    Equiv   = 9,
    Inv     = 10,
    RevOr   = 11,
    InvCopy = 12,
    InvOr   = 13,
    NAnd    = 14,
    Set     = 15
};

enum class ERglCullMode
{
    None  = 0,
    Front = 1,
    Back  = 2,
    All   = 3
};

enum class ERglAlphaFunc
{
    Never   = 0,
    Less    = 1,
    Equal   = 2,
    LEqual  = 3,
    Greater = 4,
    NEqual  = 5,
    GEqual  = 6,
    Always  = 7
};

enum class ERglAlphaOp
{
    And  = 0,
    Or   = 1,
    Xor  = 2,
    XNor = 3
};

enum class ERglFogMode
{
    None         = 0x00,

    PerspLin     = 0x02,
    PerspExp     = 0x04,
    PerspExp2    = 0x05,
    PerspRevExp  = 0x06,
    PerspRevExp2 = 0x07,

    OrthoLin     = 0x0A,
    OrthoExp     = 0x0C,
    OrthoExp2    = 0x0D,
    OrthoRevExp  = 0x0E,
    OrthoRevExp2 = 0x0F
};

struct SClipScreenRect
{
    bool x0_valid;
    int x4_left;
    int x8_top;
    int xc_width;
    int x10_height;
    int x14_dstWidth;
    float x18_uvXMin;
    float x1c_uvXMax;
    float x20_uvYMin;
    float x24_uvYMax;
};

enum class ETexelFormat
{
    I4      = 0,
    I8      = 1,
    IA4     = 2,
    IA8     = 3,
    C4      = 4,
    C8      = 5,
    C14X2   = 6,
    RGB565  = 7,
    RGB5A3  = 8,
    RGBA8   = 9,
    CMPR    = 10,
    RGBA8PC = 16,
    C8PC    = 17
};

class CGraphics
{
public:

    struct CProjectionState
    {
        bool x0_persp;
        float x4_left;
        float x8_right;
        float xc_top;
        float x10_bottom;
        float x14_near;
        float x18_far;
    };

    struct CFogState
    {
        zeus::CColor m_color;
        float m_rangeScale = 4096.f;
        float m_start = 1.f;
    };

    static CProjectionState g_Proj;
    static CFogState g_Fog;
    static float g_ProjAspect;
    static u32 g_NumLightsActive;
    static u32 g_NumBreakpointsWaiting;
    static u32 g_FlippingState;
    static bool g_LastFrameUsedAbove;
    static bool g_InterruptLastFrameUsedAbove;
    static ERglLightBits g_LightActive;
    static ERglLightBits g_LightsWereOn;
    static zeus::CTransform g_GXModelView;
    static zeus::CTransform g_GXModelViewInvXpose;
    static zeus::CTransform g_GXModelMatrix;
    static zeus::CTransform g_ViewMatrix;
    static zeus::CVector3f g_ViewPoint;
    static zeus::CTransform g_GXViewPointMatrix;
    static zeus::CTransform g_CameraMatrix;
    static zeus::CVector2i g_ViewportResolution;
    static zeus::CVector2i g_ViewportResolutionHalf;
    static int g_ViewportSamples;
    static bool g_IsGXModelMatrixIdentity;

    static void DisableAllLights();
    static void LoadLight(ERglLight light, const CLight& info);
    static void EnableLight(ERglLight light);
    static void SetLightState(ERglLightBits lightState);
    static void SetAmbientColor(const zeus::CColor& col);
    static void SetFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color);
    static void SetDepthWriteMode(bool test, ERglEnum comp, bool write);
    static void SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp);
    static void SetCullMode(ERglCullMode);
    static void BeginScene();
    static void EndScene();
    static void SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1);
    static void SetViewPointMatrix(const zeus::CTransform& xf);
    static void SetViewMatrix();
    static void SetModelMatrix(const zeus::CTransform& xf);
    static zeus::CMatrix4f CalculatePerspectiveMatrix(float fovy, float aspect,
                                                      float znear, float zfar,
                                                      bool forRenderer);
    static zeus::CMatrix4f GetPerspectiveProjectionMatrix(bool forRenderer);
    static const CProjectionState& GetProjectionState();
    static void SetProjectionState(const CProjectionState&);
    static void SetPerspective(float fovy, float aspect, float znear, float zfar);
    static void SetOrtho(float left, float right,
                         float top, float bottom,
                         float znear, float zfar);
    static void FlushProjection();
    static zeus::CVector2i ProjectPoint(const zeus::CVector3f& point);
    static SClipScreenRect ClipScreenRectFromMS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
    static SClipScreenRect ClipScreenRectFromVS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
    static zeus::CVector3f ProjectModelPointToViewportSpace(const zeus::CVector3f& point);

    static void SetViewportResolution(const zeus::CVector2i& res);
    static void SetViewport(int leftOff, int bottomOff, int width, int height);
    static void SetScissor(int leftOff, int bottomOff, int width, int height);
    static void SetDepthRange(float near, float far);

    static CTimeProvider* g_ExternalTimeProvider;
    static float g_DefaultSeconds;
    static u32 g_RenderTimings;
    static void SetExternalTimeProvider(CTimeProvider* provider)
    {g_ExternalTimeProvider = provider;}
    static float GetSecondsMod900();
    static void TickRenderTimings();

    static boo::IGraphicsDataFactory::Platform g_BooPlatform;
    static const boo::SystemChar* g_BooPlatformName;
    static boo::IGraphicsDataFactory* g_BooFactory;
    static boo::IGraphicsCommandQueue* g_BooMainCommandQueue;
    static boo::ITextureR* g_SpareTexture;

    static void InitializeBoo(boo::IGraphicsDataFactory* factory,
                              boo::IGraphicsCommandQueue* cc,
                              boo::ITextureR* spareTex)
    {
        g_BooPlatform = factory->platform();
        g_BooPlatformName = factory->platformName();
        g_BooFactory = factory;
        g_BooMainCommandQueue = cc;
        g_SpareTexture = spareTex;
    }

    const boo::SystemChar* PlatformName()
    {
        return g_BooPlatformName;
    }

    static boo::GraphicsDataToken CommitResources(const boo::FactoryCommitFunc& commitFunc)
    {
        return g_BooFactory->commitTransaction(commitFunc);
    }
    static void SetShaderDataBinding(boo::IShaderDataBinding* binding)
    {
        g_BooMainCommandQueue->setShaderDataBinding(binding);
    }
    static void ResolveSpareTexture(const SClipScreenRect& rect)
    {
        boo::SWindowRect wrect = {rect.x4_left, rect.x8_top, rect.xc_width, rect.x10_height};
        g_BooMainCommandQueue->resolveBindTexture(g_SpareTexture, wrect, true, true, false);
    }
    static void DrawInstances(size_t start, size_t count, size_t instCount)
    {
        g_BooMainCommandQueue->drawInstances(start, count, instCount);
    }
    static void DrawArray(size_t start, size_t count)
    {
        g_BooMainCommandQueue->draw(start, count);
    }
    static void DrawArrayIndexed(size_t start, size_t count)
    {
        g_BooMainCommandQueue->drawIndexed(start, count);
    }
};

}

#endif // __URDE_CGRAPHICS_HPP__
