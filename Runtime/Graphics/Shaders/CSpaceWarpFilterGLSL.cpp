#include "CSpaceWarpFilter.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform SpaceWarpUniform\n"
"{\n"
"    mat4 mainMtx;\n"
"    mat4 indMtx;\n"
"    vec4 strength;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"    vec2 indUv;\n"
"    vec2 strength;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(mat3(mainMtx) * vec3(posIn.xy, 1.0), 1.0);\n"
"    vtf.sceneUv = gl_Position.xy * vec2(0.5) + vec2(0.5);\n"
"    vtf.indUv = (mat3(indMtx) * vec3(uvIn.xy, 1.0)).xy;\n"
"    vtf.strength = strength.xy;\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"    vec2 indUv;\n"
"    vec2 strength;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"TBINDING1 uniform sampler2D indTex;\n"
"void main()\n"
"{\n"
"    colorOut = texture(sceneTex, vtf.sceneUv + (texture(indTex, vtf.indUv).xy * vec2(2.0) - vec2(1.0 - 1.0 / 256.0)) * vtf.strength.xy);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)

struct CSpaceWarpFilterGLDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CSpaceWarpFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_warpTex};
        return cctx.newShaderDataBinding(TShader<CSpaceWarpFilter>::m_pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 2, texs);
    }
};

#if BOO_HAS_VULKAN
struct CSpaceWarpFilterVulkanDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CSpaceWarpFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_warpTex};
        return cctx.newShaderDataBinding(TShader<CSpaceWarpFilter>::m_pipeline,
                                         TShader<CSpaceWarpFilter>::m_vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};
#endif

TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::GLDataFactory::Context& ctx,
                                                                             boo::IShaderPipeline*& pipeOut)
{
    const char* texNames[] = {"sceneTex", "indTex"};
    const char* uniNames[] = {"SpaceWarpUniform"};
    pipeOut = ctx.newShaderPipeline(VS, FS, 2, texNames, 1, uniNames, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CSpaceWarpFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                                                                             boo::IShaderPipeline*& pipeOut,
                                                                             boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CSpaceWarpFilterVulkanDataBindingFactory;
}
#endif

}
