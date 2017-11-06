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
"    gl_Position = mainMtx * vec4(posIn.xy, 0.0, 1.0);\n"
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

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CSpaceWarpFilterGLDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CSpaceWarpFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_warpTex.get()};
        return cctx.newShaderDataBinding(s_Pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo.get(), nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CSpaceWarpFilterVulkanDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CSpaceWarpFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_warpTex.get()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};
#endif

TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"sceneTex", "indTex"};
    const char* uniNames[] = {"SpaceWarpUniform"};
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 2, texNames, 1, uniNames, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CSpaceWarpFilterGLDataBindingFactory;
}

template <>
void CSpaceWarpFilter::Shutdown<boo::GLDataFactory>()
{
    s_Pipeline.reset();
}

#if BOO_HAS_VULKAN
TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CSpaceWarpFilterVulkanDataBindingFactory;
}

template <>
void CSpaceWarpFilter::Shutdown<boo::VulkanDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}
#endif

}
