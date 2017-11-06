#include "CColoredQuadFilter.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"\n"
"UBINDING0 uniform ColoredQuadUniform\n"
"{\n"
"    mat4 xf;\n"
"    vec4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    gl_Position = xf * vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type)
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
        return {};
    }
}

struct CColoredQuadFilterGLDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  EFilterType type, CColoredQuadFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4}
        };
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        return cctx.newShaderDataBinding(SelectPipeline(type),
                                         ctx.newVertexFormat(1, VtxVmt), filter.m_vbo.get(), nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CColoredQuadFilterVulkanDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CColoredQuadFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    }
};
#endif

TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* uniNames[] = {"ColoredQuadUniform"};
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CColoredQuadFilterGLDataBindingFactory;
}

template <>
void CColoredQuadFilter::Shutdown<boo::GLDataFactory>()
{
    s_AlphaPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CColoredQuadFilterVulkanDataBindingFactory;
}

template <>
void CColoredQuadFilter::Shutdown<boo::VulkanDataFactory>()
{
    s_VtxFmt.reset();
    s_AlphaPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
}
#endif

}
