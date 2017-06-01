#include "CRandomStaticFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform RandomStaticUniform\n"
"{\n"
"    vec4 color;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.uv = uvIn.xy;\n"
"    vtf.randOff = randOff;\n"
"    vtf.discardThres = discardThres;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"vec2i Lookup8BPP(in vec2 uv, in float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return vec2i(x, y);\n"
"}\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = texelFetch(tex, Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    colorOut.a = vtf.color.a;\n"
"}\n";

static const char* FSCookieCutter =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"vec2i Lookup8BPP(in vec2 uv, in float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return vec2i(x, y);\n"
"}\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = texelFetch(tex, Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    if (colorOut.a < vtf.discardThres)\n"
"        discard;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_AlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AddPipeline = nullptr;
static boo::IShaderPipeline* s_MultPipeline = nullptr;
static boo::IShaderPipeline* s_CookieCutterPipeline = nullptr;

static boo::IShaderPipeline* SelectPipeline(EFilterType type)
{
    switch (type)
    {
    case EFilterType::Blend:
        return s_AlphaPipeline;
    case EFilterType::Add:
        return s_AddPipeline;
    case EFilterType::Multiply:
        return s_MultPipeline;
    default:
        return nullptr;
    }
}

struct CRandomStaticFilterGLDataBindingFactory : TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CRandomStaticFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {g_Renderer->GetRandomStaticEntropyTex()};
        return cctx.newShaderDataBinding(filter.m_cookieCutter ? s_CookieCutterPipeline : SelectPipeline(type),
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CRandomStaticFilterVulkanDataBindingFactory : TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CRandomStaticFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {g_Renderer->GetRandomStaticEntropyTex()};
        return cctx.newShaderDataBinding(filter.m_cookieCutter ? s_CookieCutterPipeline : SelectPipeline(type),
                                         s_VtxFmt, filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};
#endif

TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory*
CRandomStaticFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"RandomStaticUniform"};
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_CookieCutterPipeline = ctx.newShaderPipeline(VS, FSCookieCutter, 1, texNames, 1, uniNames, boo::BlendFactor::SrcColor,
                                                   boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                                   boo::ZTest::LEqual, true, false, false, boo::CullMode::None);
    return new CRandomStaticFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory*
CRandomStaticFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_CookieCutterPipeline = ctx.newShaderPipeline(VS, FSCookieCutter, s_VtxFmt, boo::BlendFactor::SrcColor,
                                                   boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                                   boo::ZTest::LEqual, true, false, false, boo::CullMode::None);
    return new CRandomStaticFilterVulkanDataBindingFactory;
}
#endif

}
