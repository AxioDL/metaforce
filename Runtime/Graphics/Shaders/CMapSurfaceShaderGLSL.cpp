#include "CMapSurfaceShader.hpp"
#include "TShader.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"\n"
"UBINDING0 uniform MapSurfaceUniform\n"
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

URDE_DECL_SPECIALIZE_SHADER(CMapSurfaceShader)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;

struct CMapSurfaceShaderGLDataBindingFactory : TShader<CMapSurfaceShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CMapSurfaceShader& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, filter.m_ibo, boo::VertexSemantic::Position4}
        };
        boo::IVertexFormat* vtxFmt = ctx.newVertexFormat(1, VtxVmt);
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        filter.m_dataBind = cctx.newShaderDataBinding(s_Pipeline,
            vtxFmt, filter.m_vbo, nullptr, filter.m_ibo,
            1, bufs, stages, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBind;
    }
};

#if BOO_HAS_VULKAN
struct CMapSurfaceShaderVulkanDataBindingFactory : TShader<CMapSurfaceShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CMapSurfaceShader& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        filter.m_dataBind = cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
            filter.m_vbo, nullptr, filter.m_ibo, 1, bufs,
            nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBind;
    }
};
#endif

TShader<CMapSurfaceShader>::IDataBindingFactory*
CMapSurfaceShader::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* uniNames[] = {"MapSurfaceUniform"};
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames,
        boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
        boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true,
        false, boo::CullMode::Backface);
    return new CMapSurfaceShaderGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TShader<CMapSurfaceShader>::IDataBindingFactory*
CMapSurfaceShader::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
        boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
        boo::ZTest::LEqual, false, true, false, boo::CullMode::Backface);
    return new CMapSurfaceShaderVulkanDataBindingFactory;
}
#endif

}
