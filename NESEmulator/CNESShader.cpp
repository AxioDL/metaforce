#include "CNESShader.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde::MP1
{

boo::ObjToken<boo::IShaderPipeline> CNESShader::g_Pipeline;
boo::ObjToken<boo::IVertexFormat> CNESShader::g_VtxFmt;

static const char* VS_GLSL =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform TexuredQuadUniform\n"
"{\n"
"    mat4 mtx;\n"
"    vec4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.uv = uvIn.xy;\n"
"    gl_Position = mtx * vec4(posIn.xyz, 1.0);\n"
"    gl_Position = FLIPFROMGL(gl_Position);\n"
"}\n";

static const char* FS_GLSL =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(tex, vtf.uv);\n"
"}\n";

#if _WIN32
static const char* VS_HLSL =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer TexuredQuadUniform : register(b0)\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* FS_HLSL =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s4);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex.Sample(samp, vtf.uv);\n"
"}\n";
#endif

#if BOO_HAS_METAL
static const char* VS_METAL =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct TexuredQuadUniform\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";


static const char* FS_METAL =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler clampSamp [[ sampler(4) ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(clampSamp, vtf.uv);\n"
"}\n";
#endif

#if BOO_HAS_GL
void CNESShader::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TexuredQuadUniform"};
    g_Pipeline = ctx.newShaderPipeline(VS_GLSL, FS_GLSL, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
}

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::GLDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {vbo.get(), nullptr, boo::VertexSemantic::Position4},
        {vbo.get(), nullptr, boo::VertexSemantic::UV4}
    };
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    boo::ObjToken<boo::ITexture> texs[] = {tex.get()};
    return ctx.newShaderDataBinding(g_Pipeline,
                                    ctx.newVertexFormat(2, VtxVmt), vbo.get(), nullptr, nullptr,
                                    1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
}
#endif

#if _WIN32
void CNESShader::Initialize(boo::D3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    g_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    g_Pipeline = ctx.newShaderPipeline(VS_HLSL, FS_HLSL, nullptr, nullptr, nullptr,
                                       g_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
}

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::D3DDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex)
{
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uniBuf.get()};
    boo::ObjToken<boo::ITexture> texs[] = {tex.get()};
    return ctx.newShaderDataBinding(g_Pipeline, g_VtxFmt,
                                    vbo.get(), nullptr, nullptr, 1, bufs,
                                    nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
}
#endif

#if BOO_HAS_METAL
void CNESShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    g_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    g_Pipeline = ctx.newShaderPipeline(VS_METAL, FS_METAL, nullptr, nullptr,
                                       g_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
}

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::MetalDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex)
{
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uniBuf.get()};
    boo::ObjToken<boo::ITexture> texs[] = {tex.get()};
    return ctx.newShaderDataBinding(g_Pipeline, g_VtxFmt,
                                    vbo.get(), nullptr, nullptr, 1, bufs,
                                    nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
}
#endif

#if BOO_HAS_VULKAN
void CNESShader::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    g_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    g_Pipeline = ctx.newShaderPipeline(VS_GLSL, FS_GLSL, g_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
}

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::VulkanDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex)
{
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uniBuf.get()};
    boo::ObjToken<boo::ITexture> texs[] = {tex.get()};
    return ctx.newShaderDataBinding(g_Pipeline, g_VtxFmt,
                                    vbo.get(), nullptr, nullptr, 1, bufs,
                                    nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
}
#endif

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex)
{
    switch (ctx.platform())
    {
#if BOO_HAS_GL
    case boo::IGraphicsDataFactory::Platform::OpenGL:
        return BuildShaderDataBinding(static_cast<boo::GLDataFactory::Context&>(ctx), vbo, uniBuf, tex);
#endif
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
        return BuildShaderDataBinding(static_cast<boo::D3DDataFactory::Context&>(ctx), vbo, uniBuf, tex);
#endif
#if BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        return BuildShaderDataBinding(static_cast<boo::MetalDataFactory::Context&>(ctx), vbo, uniBuf, tex);
#endif
#if BOO_HAS_VULKAN
    case boo::IGraphicsDataFactory::Platform::Vulkan:
        return BuildShaderDataBinding(static_cast<boo::VulkanDataFactory::Context&>(ctx), vbo, uniBuf, tex);
#endif
    default:
        return {};
    }
}

void CNESShader::Initialize()
{
    if (!CGraphics::g_BooFactory)
        return;

    CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx)
    {
        switch (ctx.platform())
        {
#if BOO_HAS_GL
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            Initialize(static_cast<boo::GLDataFactory::Context&>(ctx));
            break;
#endif
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
            Initialize(static_cast<boo::D3DDataFactory::Context&>(ctx));
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            Initialize(static_cast<boo::MetalDataFactory::Context&>(ctx));
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            Initialize(static_cast<boo::VulkanDataFactory::Context&>(ctx));
            break;
#endif
        default: break;
        }
        return true;
    } BooTrace);
}

void CNESShader::Shutdown()
{
    g_Pipeline.reset();
    g_VtxFmt.reset();
}

}
