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
"    vtf.uv.y = 1.0 - vtf.uv.y;\n"
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
"    vtf.uv = uvIn.xy;\n"
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

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaFlipPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaGEqualFlipPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddFlipPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultFlipPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type, bool gequal, bool flip)
{
    if (gequal)
        return flip ? s_AlphaGEqualFlipPipeline : s_AlphaGEqualPipeline;
    switch (type)
    {
    case EFilterType::Blend:
        return flip ? s_AlphaFlipPipeline : s_AlphaPipeline;
    case EFilterType::Add:
        return flip ? s_AddFlipPipeline : s_AddPipeline;
    case EFilterType::Multiply:
        return flip ? s_MultFlipPipeline : s_MultPipeline;
    default:
        return {};
    }
}

static boo::ObjToken<boo::IVertexFormat> s_AVtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AAlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AAddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AMultPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectAlphaPipeline(EFilterType type)
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
        return {};
    }
}

struct CTexturedQuadFilterGLDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  EFilterType type, CTexturedQuadFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal,
                                                        filter.m_booTex->type() == boo::TextureType::Render),
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo.get(), nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CTexturedQuadFilterVulkanDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CTexturedQuadFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal,
                                                        filter.m_booTex->type() == boo::TextureType::Render), s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
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
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaGEqualFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                  boo::ZTest::GEqual, true, true, false, boo::CullMode::None);
    s_AddFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, 1, texNames, 1, uniNames, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterGLDataBindingFactory;
}

template <>
void CTexturedQuadFilter::Shutdown<boo::GLDataFactory>()
{
    s_AlphaPipeline.reset();
    s_AlphaGEqualPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
    s_AlphaFlipPipeline.reset();
    s_AlphaGEqualFlipPipeline.reset();
    s_AddFlipPipeline.reset();
    s_MultFlipPipeline.reset();
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
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaGEqualFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                  boo::ZTest::GEqual, true, true, false, boo::CullMode::None);
    s_AddFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultFlipPipeline = ctx.newShaderPipeline(VSFlip, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterVulkanDataBindingFactory;
}

template <>
void CTexturedQuadFilter::Shutdown<boo::VulkanDataFactory>()
{
    s_VtxFmt.reset();
    s_AlphaPipeline.reset();
    s_AlphaGEqualPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
    s_AlphaFlipPipeline.reset();
    s_AlphaGEqualFlipPipeline.reset();
    s_AddFlipPipeline.reset();
    s_MultFlipPipeline.reset();
}
#endif

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilterAlpha)

struct CTexturedQuadFilterAlphaGLDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  EFilterType type, CTexturedQuadFilterAlpha& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type),
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo.get(), nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CTexturedQuadFilterAlphaVulkanDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CTexturedQuadFilterAlpha& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type), s_AVtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};
#endif

TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TexuredQuadUniform"};
    s_AAlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, 1, texNames, 1, uniNames, boo::BlendFactor::Zero,
                                            boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaGLDataBindingFactory;
}

template <>
void CTexturedQuadFilterAlpha::Shutdown<boo::GLDataFactory>()
{
    s_AAlphaPipeline.reset();
    s_AAddPipeline.reset();
    s_AMultPipeline.reset();
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
    s_AAlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSNoFlip, FSAlpha, s_AVtxFmt, boo::BlendFactor::Zero,
                                            boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, true, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaVulkanDataBindingFactory;
}

template <>
void CTexturedQuadFilterAlpha::Shutdown<boo::VulkanDataFactory>()
{
    s_AVtxFmt.reset();
    s_AAlphaPipeline.reset();
    s_AAddPipeline.reset();
    s_AMultPipeline.reset();
}
#endif

}
