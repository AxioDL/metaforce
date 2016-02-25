#ifndef __PSHAG_CGRAPHICS_HPP__
#define __PSHAG_CGRAPHICS_HPP__

#include "RetroTypes.hpp"
#include "CTransform.hpp"
#include "CVector2i.hpp"

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/graphicsdev/IGraphicsCommandQueue.hpp"

namespace pshag
{

enum class ERglLight : u8
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
ENABLE_BITWISE_ENUM(ERglLight)

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
    I4     = 0,
    I8     = 1,
    IA4    = 2,
    IA8    = 3,
    C4     = 4,
    C8     = 5,
    C14X2  = 6,
    RGB565 = 7,
    RGB5A3 = 8,
    RGBA8  = 9,
    CMPR   = 10
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
    static CProjectionState g_Proj;
    static float g_ProjAspect;
    static u32 g_NumLightsActive;
    static ERglLight g_LightActive;
    static ERglLight g_LightsWereOn;
    static Zeus::CTransform g_GXModelView;
    static Zeus::CTransform g_GXModelMatrix;
    static Zeus::CTransform g_ViewMatrix;
    static Zeus::CVector3f g_ViewPoint;
    static Zeus::CTransform g_GXViewPointMatrix;
    static Zeus::CTransform g_CameraMatrix;
    static Zeus::CVector2i g_ViewportResolution;
    static Zeus::CVector2i g_ViewportResolutionHalf;
    static int g_ViewportSamples;
    static bool g_IsGXModelMatrixIdentity;

    static void DisableAllLights();
    static void EnableLight(ERglLight light);
    static void SetLightState(ERglLight lightState);
    static void SetDepthWriteMode(bool test, ERglEnum comp, bool write);
    static void SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp);
    static void SetCullMode(ERglCullMode);
    static void SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1);
    static void SetViewPointMatrix(const Zeus::CTransform& xf);
    static void SetViewMatrix();
    static void SetModelMatrix(const Zeus::CTransform& xf);
    static Zeus::CMatrix4f GetPerspectiveProjectionMatrix();
    static const CProjectionState& GetProjectionState();
    static void SetProjectionState(const CProjectionState&);
    static void SetPerspective(float fovy, float aspect, float near, float far);
    static void FlushProjection();
    static Zeus::CVector2i ProjectPoint(const Zeus::CVector3f& point);
    static SClipScreenRect ClipScreenRectFromMS(const Zeus::CVector3f& p1, const Zeus::CVector3f& p2);
    static SClipScreenRect ClipScreenRectFromVS(const Zeus::CVector3f& p1, const Zeus::CVector3f& p2);
    static Zeus::CVector3f ProjectModelPointToViewportSpace(const Zeus::CVector3f& point);
    static void SetViewportResolution(const Zeus::CVector2i& res);

    static boo::IGraphicsDataFactory* g_BooFactory;
    static boo::IGraphicsCommandQueue* g_BooMainCommandQueue;
    static boo::ITextureR* g_SpareTexture;

    static void InitializeBoo(boo::IGraphicsDataFactory* factory,
                              boo::IGraphicsCommandQueue* cc,
                              boo::ITextureR* spareTex)
    {
        g_BooFactory = factory;
        g_BooMainCommandQueue = cc;
        g_SpareTexture = spareTex;
    }

    static boo::IGraphicsBufferD* NewDynamicGPUBuffer(boo::BufferUse use, size_t stride, size_t count)
    {
        return g_BooFactory->newDynamicBuffer(use, stride, count);
    }
    static boo::GraphicsDataToken CommitResources()
    {
        return g_BooFactory->commit();
    }
    static void SetShaderDataBinding(boo::IShaderDataBinding* binding)
    {
        g_BooMainCommandQueue->setShaderDataBinding(binding);
    }
    static void ResolveSpareTexture(const SClipScreenRect& rect, bool color, bool depth)
    {
        boo::SWindowRect wrect = {rect.x4_left, rect.x8_top, rect.xc_width, rect.x10_height};
        g_BooMainCommandQueue->resolveBindTexture(g_SpareTexture, wrect, true, color, depth);
    }
    static void DrawInstances(size_t start, size_t count, size_t instCount)
    {
        g_BooMainCommandQueue->drawInstances(start, count, instCount);
    }
    static void DrawArray(size_t start, size_t count)
    {
        g_BooMainCommandQueue->draw(start, count);
    }
};

}

#endif // __PSHAG_CGRAPHICS_HPP__
