#include "CXRayBlurFilter.hpp"
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
"UBINDING0 uniform XRayBlurUniform\n"
"{\n"
"    mat4 uv0;\n"
"    mat4 uv1;\n"
"    mat4 uv2;\n"
"    mat4 uv3;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 uv0;\n"
"    vec2 uv1;\n"
"    vec2 uv2;\n"
"    vec2 uv3;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.uv0 = (uv0 * vec4(uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv1 = (uv1 * vec4(uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv2 = (uv2 * vec4(uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv3 = (uv3 * vec4(uvIn.xy, 0.0, 1.0)).xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 uv0;\n"
"    vec2 uv1;\n"
"    vec2 uv2;\n"
"    vec2 uv3;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"TBINDING1 uniform sampler2D paletteTex;\n"
"const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);\n"
"void main()\n"
"{\n"
"    vec4 colorSample = texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorOut = colorSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CXRayBlurFilterGLDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CXRayBlurFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_booTex};
        return cctx.newShaderDataBinding(s_Pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo.get(), nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CXRayBlurFilterVulkanDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CXRayBlurFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_booTex};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};
#endif

TShader<CXRayBlurFilter>::IDataBindingFactory* CXRayBlurFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"sceneTex", "paletteTex"};
    const char* uniNames[] = {"XRayBlurUniform"};
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 2, texNames, 1, uniNames, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CXRayBlurFilterGLDataBindingFactory;
}

template <>
void CXRayBlurFilter::Shutdown<boo::GLDataFactory>()
{
    s_Pipeline.reset();
}

#if BOO_HAS_VULKAN
TShader<CXRayBlurFilter>::IDataBindingFactory* CXRayBlurFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
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
    return new CXRayBlurFilterVulkanDataBindingFactory;
}
#endif

}
