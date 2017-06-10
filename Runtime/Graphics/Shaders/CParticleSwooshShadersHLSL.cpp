#include "CParticleSwooshShaders.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CSwooshDescription.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer SwooshUniform : register(b0)\n"
"{\n"
"    float4x4 mvp;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uv = v.uvIn;\n"
"    vtf.pos = mul(mvp, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* FS_TEX =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"SamplerState samp : register(s0);\n"
"Texture2D tex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex.Sample(samp, vtf.uv);\n"
"}\n";

static const char* FS_NOTEX =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color;\n"
"}\n";

struct D3DParticleSwooshDataBindingFactory : TShader<CParticleSwooshShaders>::IDataBindingFactory
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

TShader<CParticleSwooshShaders>::IDataBindingFactory* CParticleSwooshShaders::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor VtxFmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4},
        {nullptr, nullptr, boo::VertexSemantic::Color},
    };
    m_vtxFormat = ctx.newVertexFormat(3, VtxFmt);

    m_texZWrite = ctx.newShaderPipeline(VS, FS_TEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                        boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                        boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                        true, false, boo::CullMode::None);
    m_texNoZWrite = ctx.newShaderPipeline(VS, FS_TEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                          boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                          true, false, boo::CullMode::None);
    m_texAdditiveZWrite = ctx.newShaderPipeline(VS, FS_TEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                                true, false, boo::CullMode::None);
    m_texAdditiveNoZWrite = ctx.newShaderPipeline(VS, FS_TEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                  true, false, boo::CullMode::None);

    m_noTexZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                          boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                          true, false, boo::CullMode::None);
    m_noTexNoZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                            boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                            true, false, boo::CullMode::None);
    m_noTexAdditiveZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                                  true, false, boo::CullMode::None);
    m_noTexAdditiveNoZWrite = ctx.newShaderPipeline(VS, FS_NOTEX, nullptr, nullptr, nullptr, m_vtxFormat,
                                                    boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                    boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                    true, false, boo::CullMode::None);

    return new struct D3DParticleSwooshDataBindingFactory;
}

}
