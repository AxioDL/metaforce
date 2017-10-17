#include "CParticleSwooshShaders.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CSwooshDescription.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"    float4 colorIn [[ attribute(2) ]];\n"
"};\n"
"\n"
"struct SwooshUniform\n"
"{\n"
"    float4x4 mvp;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SwooshUniform& su [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.pos = su.mvp * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS_TEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv);\n"
"}\n";

static const char* FS_NOTEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
"{\n"
"    return vtf.color;\n"
"}\n";

struct MetalParticleSwooshDataBindingFactory : TShader<CParticleSwooshShaders>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CParticleSwooshShaders& shaders)
    {
        CParticleSwoosh& gen = shaders.m_gen;
        CSwooshDescription* desc = gen.GetDesc();

        CUVElement* texr = desc->x3c_TEXR.get();
        boo::ITexture* textures[] = {texr->GetValueTexture(0).GetObj()->GetBooTexture()};

        boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBuf};
        gen.m_dataBind = ctx.newShaderDataBinding(shaders.m_pipeline, CParticleSwooshShaders::m_vtxFormat,
                                                  gen.m_vertBuf, nullptr, nullptr, 1, uniforms,
                                                  nullptr, texr ? 1 : 0, textures, nullptr, nullptr);
        return nullptr;
    }
};

TShader<CParticleSwooshShaders>::IDataBindingFactory* CParticleSwooshShaders::Initialize(boo::MetalDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor VtxFmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4},
        {nullptr, nullptr, boo::VertexSemantic::Color},
    };
    m_vtxFormat = ctx.newVertexFormat(3, VtxFmt);

    m_texZWrite = ctx.newShaderPipeline(VS, FS_TEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                        boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                        boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                        true, false, boo::CullMode::None);
    m_texNoZWrite = ctx.newShaderPipeline(VS, FS_TEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                          boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                          true, false, boo::CullMode::None);
    m_texAdditiveZWrite = ctx.newShaderPipeline(VS, FS_TEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                                true, false, boo::CullMode::None);
    m_texAdditiveNoZWrite = ctx.newShaderPipeline(VS, FS_TEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                  true, false, boo::CullMode::None);

    m_noTexZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                          boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                          true, false, boo::CullMode::None);
    m_noTexNoZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                            boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                            true, false, boo::CullMode::None);
    m_noTexAdditiveZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                                  true, false, boo::CullMode::None);
    m_noTexAdditiveNoZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, m_vtxFormat, CGraphics::g_ViewportSamples,
                                                    boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                    boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                    true, false, boo::CullMode::None);

    return new struct MetalParticleSwooshDataBindingFactory;
}

}
