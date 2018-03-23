#include "CDecalShaders.hpp"
#include "Particle/CDecal.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{

static const char* VS_HLSL_TEX =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"    float4 uvsIn[4] : UV;\n"
"};\n"
"\n"
"cbuffer DecalUniform : register(b0)\n"
"{\n"
"    float4x4 mvp;\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn * moduColor;\n"
"    vtf.uv = v.uvsIn[vertId].xy;\n"
"    vtf.position = mul(mvp, v.posIn[vertId]);\n"
"    return vtf;\n"
"}\n";

static const char* FS_HLSL_TEX =
"SamplerState samp : register(s0);\n"
"Texture2D tex0 : register(t0);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex0.Sample(samp, vtf.uv);\n"
"}\n";

static const char* FS_HLSL_TEX_REDTOALPHA =
"SamplerState samp : register(s0);\n"
"Texture2D tex0 : register(t0);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return float4(vtf.color.rgb, tex0.Sample(samp, vtf.uv).r);\n"
"}\n";

static const char* VS_HLSL_NOTEX =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer DecalUniform : register(b0)\n"
"{\n"
"    float4x4 mvp;\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn * moduColor;\n"
"    vtf.position = mul(mvp, v.posIn[vertId]);\n"
"    return vtf;\n"
"}\n";

static const char* FS_HLSL_NOTEX =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color;\n"
"}\n";

struct D3DDecalDataBindingFactory : TShader<CDecalShaders>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CDecalShaders& shaders)
    {
        CQuadDecal& decal = shaders.m_decal;
        const SQuadDescr* desc = decal.m_desc;

        CUVElement* texr = desc->x14_TEX.get();
        int texCount = 0;
        boo::ObjToken<boo::ITexture> textures[1];

        if (texr)
        {
            textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
            texCount = 1;
        }

        if (decal.m_instBuf)
        {
            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {decal.m_uniformBuf.get()};

            if (shaders.m_regPipeline)
                decal.m_normalDataBind = ctx.newShaderDataBinding(shaders.m_regPipeline, nullptr, nullptr,
                                                                decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                                nullptr, texCount, textures,
                                                                nullptr, nullptr);
            if (shaders.m_redToAlphaPipeline)
                decal.m_redToAlphaDataBind = ctx.newShaderDataBinding(shaders.m_redToAlphaPipeline, nullptr, nullptr,
                                                                    decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                                    nullptr, texCount, textures,
                                                                    nullptr, nullptr);
        }

        return nullptr;
    }
};

TShader<CDecalShaders>::IDataBindingFactory* CDecalShaders::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor TexFmtTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
    };
    m_vtxFormatTex = ctx.newVertexFormat(9, TexFmtTex);

    static const boo::VertexElementDescriptor TexFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFormatNoTex = ctx.newVertexFormat(5, TexFmtNoTex);

    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, nullptr, nullptr,
                                               nullptr, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texAdditiveZTest = ctx.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, nullptr, nullptr,
                                               nullptr, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX_REDTOALPHA, nullptr, nullptr,
                                                 nullptr, m_vtxFormatTex,
                                                 boo::BlendFactor::One, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, true, boo::CullMode::None);
    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, nullptr, nullptr,
                                                 nullptr, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, nullptr, nullptr,
                                                 nullptr, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                                 true, false, boo::CullMode::None);

    return new struct D3DDecalDataBindingFactory;
}

template <>
void CDecalShaders::Shutdown<boo::ID3DDataFactory>()
{
    m_vtxFormatTex.reset();
    m_vtxFormatNoTex.reset();

    m_texZTestNoZWrite.reset();
    m_texAdditiveZTest.reset();
    m_texRedToAlphaZTest.reset();

    m_noTexZTestNoZWrite.reset();
    m_noTexAdditiveZTest.reset();
}

}
