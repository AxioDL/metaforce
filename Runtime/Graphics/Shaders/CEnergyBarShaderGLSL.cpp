#include "CEnergyBarShader.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform EnergyBarUniform\n"
"{\n"
"    mat4 xf;\n"
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
"    gl_Position = xf * vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
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

URDE_DECL_SPECIALIZE_SHADER(CEnergyBarShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CEnergyBarShaderGLDataBindingFactory : TShader<CEnergyBarShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CEnergyBarShader& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IVertexFormat> vtxFmt = ctx.newVertexFormat(2, VtxVmt);
        boo::ObjToken<boo::IGraphicsBuffer> bufs[1];
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_tex->GetBooTexture()};
        for (int i=0 ; i<3 ; ++i)
        {
            bufs[0] = filter.m_uniBuf[i].get();
            filter.m_dataBind[i] = cctx.newShaderDataBinding(s_Pipeline,
                vtxFmt, filter.m_vbo.get(), nullptr, nullptr,
                1, bufs, stages, nullptr, nullptr, 1, texs, nullptr, nullptr);
        }
        return filter.m_dataBind[0];
    }
};

#if BOO_HAS_VULKAN
struct CEnergyBarShaderVulkanDataBindingFactory : TShader<CEnergyBarShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CEnergyBarShader& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[1];
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_tex->GetBooTexture()};
        for (int i=0 ; i<3 ; ++i)
        {
            bufs[0] = filter.m_uniBuf[i].get();
            filter.m_dataBind[i] = cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
        }
        return filter.m_dataBind[0];
    }
};
#endif

TShader<CEnergyBarShader>::IDataBindingFactory*
CEnergyBarShader::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* uniNames[] = {"EnergyBarUniform"};
    const char* texNames[] = {"tex"};
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    return new CEnergyBarShaderGLDataBindingFactory;
}

template <>
void CEnergyBarShader::Shutdown<boo::GLDataFactory>()
{
    s_Pipeline.reset();
}

#if BOO_HAS_VULKAN
TShader<CEnergyBarShader>::IDataBindingFactory*
CEnergyBarShader::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    return new CEnergyBarShaderVulkanDataBindingFactory;
}

template <>
void CEnergyBarShader::Shutdown<boo::VulkanDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}
#endif

}
