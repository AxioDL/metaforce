#include "CFogVolumeFilter.hpp"
#include "TShader.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec2 uvIn;\n"
"\n"
"UBINDING0 uniform FogVolumeFilterUniform\n"
"{\n"
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
"    gl_Position = vec4(posIn.xy, 0.0, 1.0);\n"
"    vtf.color = color;\n"
"    vtf.uv = uvIn;\n"
"}\n";

static const char* FS1Way =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D zFrontfaceTex;\n"
"TBINDING1 uniform sampler2D zBackfaceTex;\n"
"TBINDING2 uniform sampler2D zLinearizer;\n"
"void main()\n"
"{\n"
"    float y;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float x = modf(texture(zFrontfaceTex, vtf.uv).r * linScale, y);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float alpha = texture(zLinearizer, vec2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;\n"
"    colorOut = vtf.color * alpha;\n"
"}\n";

static const char* FS2Way =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D zFrontfaceTex;\n"
"TBINDING1 uniform sampler2D zBackfaceTex;\n"
"TBINDING2 uniform sampler2D zLinearizer;\n"
"void main()\n"
"{\n"
"    float frontY;\n"
"    float backY;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float frontX = modf(texture(zFrontfaceTex, vtf.uv).r * linScale, frontY);\n"
"    float backX = modf(texture(zBackfaceTex, vtf.uv).r * linScale, backY);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float frontLin = texture(zLinearizer, vec2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;\n"
"    float backLin = texture(zLinearizer, vec2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;\n"
"    colorOut = vec4(vtf.color.rgb, (frontLin - backLin) * 10.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumeFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_1WayPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_2WayPipeline;

struct CFogVolumeFilterGLDataBindingFactory : TShader<CFogVolumeFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CFogVolumeFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IVertexFormat> VtxVmtObj = cctx.newVertexFormat(2, VtxVmt);
        boo::ObjToken<boo::ITexture> texs[] = { CGraphics::g_SpareTexture.get(), CGraphics::g_SpareTexture.get(),
                                                g_Renderer->GetFogRampTex().get() };
        int bindIdxs[] = {0, 1, 0};
        bool bindDepth[] = {true, true, false};
        boo::ObjToken<boo::IGraphicsBuffer> ubufs[] = {filter.m_uniBuf.get()};

        filter.m_dataBind1Way = cctx.newShaderDataBinding(s_1WayPipeline, VtxVmtObj,
            filter.m_vbo.get(), nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        filter.m_dataBind2Way = cctx.newShaderDataBinding(s_2WayPipeline, VtxVmtObj,
            filter.m_vbo.get(), nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        return filter.m_dataBind1Way;
    }
};

#if BOO_HAS_VULKAN
struct CFogVolumeFilterVulkanDataBindingFactory : TShader<CFogVolumeFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CFogVolumeFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);
        boo::ITexture* texs[] = { CGraphics::g_SpareTexture, CGraphics::g_SpareTexture,
                                  g_Renderer->GetFogRampTex() };
        int bindIdxs[] = {0, 1, 0};
        bool bindDepth[] = {true, true, false};
        boo::IGraphicsBuffer* ubufs[] = {filter.m_uniBuf};

        filter.m_dataBind1Way = cctx.newShaderDataBinding(s_1WayPipeline, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        filter.m_dataBind2Way = cctx.newShaderDataBinding(s_2WayPipeline, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        return filter.m_dataBind1Way;
    }
};
#endif

TShader<CFogVolumeFilter>::IDataBindingFactory*
CFogVolumeFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"zFrontfaceTex", "zBackfaceTex", "zLinearizer"};
    const char* uniNames[] = {"FogVolumeFilterUniform"};
    s_1WayPipeline = ctx.newShaderPipeline(VS, FS1Way, 3, texNames, 1, uniNames, boo::BlendFactor::DstAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_2WayPipeline = ctx.newShaderPipeline(VS, FS2Way, 3, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CFogVolumeFilterGLDataBindingFactory;
}

template <>
void CFogVolumeFilter::Shutdown<boo::GLDataFactory>()
{
    s_1WayPipeline.reset();
    s_2WayPipeline.reset();
}

#if BOO_HAS_VULKAN
TShader<CFogVolumeFilter>::IDataBindingFactory*
CFogVolumeFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_1WayPipeline = ctx.newShaderPipeline(VS, FS1Way, s_VtxFmt, boo::BlendFactor::DstAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_2WayPipeline = ctx.newShaderPipeline(VS, FS2Way, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CFogVolumeFilterVulkanDataBindingFactory;
}
#endif

}
