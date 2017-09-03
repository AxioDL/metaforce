#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VSFlip =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform TexuredQuadUniform\n"
"{\n"
"    mat4 mtx;\n"
"    vec4 color;\n"
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.uv = uvIn.xy;\n"
"    vtf.lod = lod;\n"
"    gl_Position = mtx * vec4(posIn.xyz, 1.0);\n"
"    gl_Position = FLIPFROMGL(gl_Position);\n"
"}\n";

static const char* VSNoFlip =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform TexuredQuadUniform\n"
"{\n"
"    mat4 mtx;\n"
"    vec4 color;\n"
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.uv = -uvIn.xy;\n"
"    vtf.lod = lod;\n"
"    gl_Position = mtx * vec4(posIn.xyz, 1.0);\n"
"    gl_Position = FLIPFROMGL(gl_Position);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * vec4(texture(tex, vtf.uv, vtf.lod).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(tex, vtf.uv, vtf.lod);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_AlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AlphaGEqualPipeline = nullptr;
static boo::IShaderPipeline* s_AddPipeline = nullptr;
static boo::IShaderPipeline* s_MultPipeline = nullptr;

static boo::IShaderPipeline* SelectPipeline(EFilterType type, bool gequal)
{
    if (gequal)
        return s_AlphaGEqualPipeline;
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

static boo::IVertexFormat* s_AVtxFmt = nullptr;
static boo::IShaderPipeline* s_AAlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AAddPipeline = nullptr;
static boo::IShaderPipeline* s_AMultPipeline = nullptr;

static boo::IShaderPipeline* SelectAlphaPipeline(EFilterType type)
{
    switch (type)
    {
    case EFilterType::Blend:
        return s_AAlphaPipeline;
    case EFilterType::Add:
        return s_AAddPipeline;
    case EFilterType::Multiply:
        return s_AMultPipeline;
    default:
        return nullptr;
    }
}

struct CTexturedQuadFilterGLDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CTexturedQuadFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal),
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CTexturedQuadFilterVulkanDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CTexturedQuadFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal), s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};
#endif

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TexuredQuadUniform"};
    s_AlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaGEqualPipeline = ctx.newShaderPipeline(VSNoFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                  boo::ZTest::GEqual, true, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaGEqualPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                  boo::ZTest::GEqual, true, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterVulkanDataBindingFactory;
}
#endif

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilterAlpha)

struct CTexturedQuadFilterAlphaGLDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CTexturedQuadFilterAlpha& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type),
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CTexturedQuadFilterAlphaVulkanDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CTexturedQuadFilterAlpha& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type), s_AVtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};
#endif

TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TexuredQuadUniform"};
    s_AAlphaPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::SrcColor,
                                            boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_AVtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AAlphaPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::SrcColor,
                                            boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, true, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaVulkanDataBindingFactory;
}
#endif

}
