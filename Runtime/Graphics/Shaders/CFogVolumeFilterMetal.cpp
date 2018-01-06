#include "CFogVolumeFilter.hpp"
#include "TShader.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float2 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct FogVolumeFilterUniform\n"
"{\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]],\n"
"                        constant FogVolumeFilterUniform& fu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.pos = float4(v.posIn.x, -v.posIn.y, 0.0, 1.0);\n"
"    vtf.color = fu.color;\n"
"    vtf.uv = v.uvIn;\n"
"    return vtf;\n"
"}\n";

static const char* FS1Way =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> zFrontfaceTex [[ texture(0) ]],\n"
"                      texture2d<float> zBackfaceTex [[ texture(1) ]],\n"
"                      texture2d<float> zLinearizer [[ texture(2) ]])\n"
"{\n"
"    float y;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float x = modf((1.0 - zFrontfaceTex.sample(samp, vtf.uv).r) * linScale, y);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float alpha = zLinearizer.sample(samp, float2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;\n"
"    return vtf.color * alpha;\n"
"}\n";

static const char* FS2Way =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> zFrontfaceTex [[ texture(0) ]],\n"
"                      texture2d<float> zBackfaceTex [[ texture(1) ]],\n"
"                      texture2d<float> zLinearizer [[ texture(2) ]])\n"
"{\n"
"    float frontY;\n"
"    float backY;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float frontX = modf((1.0 - zFrontfaceTex.sample(samp, vtf.uv).r) * linScale, frontY);\n"
"    float backX = modf((1.0 - zBackfaceTex.sample(samp, vtf.uv).r) * linScale, backY);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float frontLin = zLinearizer.sample(samp, float2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;\n"
"    float backLin = zLinearizer.sample(samp, float2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;\n"
"    return float4(vtf.color.rgb, (frontLin - backLin) * 10.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumeFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_1WayPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_2WayPipeline;

struct CFogVolumeFilterMetalDataBindingFactory : TShader<CFogVolumeFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CFogVolumeFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);
        boo::ObjToken<boo::ITexture> texs[] = { CGraphics::g_SpareTexture.get(), CGraphics::g_SpareTexture.get(),
                                                g_Renderer->GetFogRampTex().get() };
        int bindIdxs[] = {0, 1, 0};
        bool bindDepth[] = {true, true, false};
        boo::ObjToken<boo::IGraphicsBuffer> ubufs[] = {filter.m_uniBuf.get()};

        filter.m_dataBind1Way = cctx.newShaderDataBinding(s_1WayPipeline, s_VtxFmt,
            filter.m_vbo.get(), nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        filter.m_dataBind2Way = cctx.newShaderDataBinding(s_2WayPipeline, s_VtxFmt,
            filter.m_vbo.get(), nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        return filter.m_dataBind1Way;
    }
};

TShader<CFogVolumeFilter>::IDataBindingFactory*
CFogVolumeFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_1WayPipeline = ctx.newShaderPipeline(VS, FS1Way, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::DstAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_2WayPipeline = ctx.newShaderPipeline(VS, FS2Way, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CFogVolumeFilterMetalDataBindingFactory;
}

template <>
void CFogVolumeFilter::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_1WayPipeline.reset();
    s_2WayPipeline.reset();
}

}
