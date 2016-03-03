#include "CElementGenShaders.hpp"
#include "CElementGen.hpp"
#include "CGenDescription.hpp"
#include "CElectricDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CModel.hpp"

namespace pshag
{

static const char* VS_HLSL_TEX =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"    float4 uvsIn[4] : UV;\n"
"};\n"
"\n"
"cbuffer ParticleUniform : register(b0)\n"
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
"    float4 colr = vtf.color * tex0.Sample(samp, vtf.uv);\n"
"    return float4(colr.rgb, colr.r);"
"}\n";

static const char* VS_HLSL_INDTEX =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"    float4 uvsInTexrTind[4] : UV0;\n"
"    float4 uvsInScene : UV4;\n"
"};\n"
"\n"
"cbuffer ParticleUniform : register(b0)\n"
"{\n"
"    float4x4 mvp;\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 uvScene : UV0;\n"
"    float2 uvTexr : UV1;\n"
"    float2 uvTind : UV2;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn * moduColor;\n"
"    vtf.uvScene = v.uvsInScene * float4(1.0, -1.0, 1.0, -1.0);\n"
"    vtf.uvTexr = v.uvsInTexrTind[vertId].xy;\n"
"    vtf.uvTind = v.uvsInTexrTind[vertId].zw;\n"
"    vtf.position = mul(mvp, v.posIn[vertId]);\n"
"    return vtf;\n"
"}\n";

static const char* FS_HLSL_INDTEX =
"SamplerState samp : register(s0);\n"
"Texture2D tex0 : register(t0);\n"
"Texture2D tex1 : register(t1);\n"
"Texture2D tex2 : register(t2);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 uvScene : UV0;\n"
"    float2 uvTexr : UV1;\n"
"    float2 uvTind : UV2;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float2 tindTexel = tex2.Sample(samp, vtf.uvTind).zw;\n"
"    float4 sceneTexel = tex1.Sample(samp, lerp(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    float4 texrTexel = tex0.Sample(samp, vtf.uvTexr);\n"
"    float4 colorOut = vtf.color * sceneTexel + texrTexel;\n"
"    colorOut.a = vtf.color.a * texrTexel.a;\n"
"    return colorOut;\n"
"}\n";

static const char* FS_HLSL_CINDTEX =
"SamplerState samp : register(s0);\n"
"Texture2D tex0 : register(t0);\n"
"Texture2D tex1 : register(t1);\n"
"Texture2D tex2 : register(t2);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 uvScene : UV0;\n"
"    float2 uvTexr : UV1;\n"
"    float2 uvTind : UV2;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float2 tindTexel = tex2.Sample(samp, vtf.uvTind).ba;\n"
"    float4 sceneTexel = tex1.Sample(samp, lerp(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    return vtf.color * sceneTexel * tex0.Sample(samp, vtf.uvTexr);\n"
"}\n";

static const char* VS_HLSL_NOTEX =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer ParticleUniform : register(b0)\n"
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

struct D3DElementDataBindingFactory : CElementGenShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(CElementGen& gen,
                                boo::IShaderPipeline* regPipeline,
                                boo::IShaderPipeline* redToAlphaPipeline)
    {
        CGenDescription* desc = gen.GetDesc();

        CUVElement* texr = desc->x54_x40_TEXR.get();
        CUVElement* tind = desc->x58_x44_TIND.get();
        int texCount = 0;
        boo::ITexture* textures[3];

        if (texr)
        {
            textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
            texCount = 1;
            if (tind)
            {
                textures[1] = CGraphics::g_SpareTexture;
                textures[2] = tind->GetValueTexture(0).GetObj()->GetBooTexture();
                texCount = 3;
            }
        }

        boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBuf};

        if (regPipeline)
            gen.m_normalDataBind = CGraphics::g_BooFactory->newShaderDataBinding(regPipeline, nullptr, nullptr,
                                                                                 gen.m_instBuf, nullptr, 1, uniforms,
                                                                                 texCount, textures);
        if (redToAlphaPipeline)
            gen.m_redToAlphaDataBind = CGraphics::g_BooFactory->newShaderDataBinding(redToAlphaPipeline, nullptr, nullptr,
                                                                                     gen.m_instBuf, nullptr, 1, uniforms,
                                                                                     texCount, textures);
    }
};

CElementGenShaders::IDataBindingFactory* CElementGenShaders::Initialize(boo::ID3DDataFactory& factory)
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
    m_vtxFormatTex = factory.newVertexFormat(9, TexFmtTex);

    static const boo::VertexElementDescriptor TexFmtIndTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 4}
    };
    m_vtxFormatIndTex = CGraphics::g_BooFactory->newVertexFormat(10, TexFmtIndTex);

    static const boo::VertexElementDescriptor TexFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFormatNoTex = CGraphics::g_BooFactory->newVertexFormat(5, TexFmtNoTex);

    m_texZTestZWrite = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                 ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 true, true, false);
    m_texNoZTestZWrite = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                   ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   false, true, false);
    m_texZTestNoZWrite = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                   ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   true, false, false);
    m_texNoZTestNoZWrite = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     false, false, false);

    m_texAdditiveZTest = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                   ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   true, false, false);
    m_texAdditiveNoZTest = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     false, false, false);

    m_texRedToAlphaZTest = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX_REDTOALPHA, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     true, false, false);
    m_texRedToAlphaNoZTest = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX_REDTOALPHA, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                       ComPtr<ID3DBlob>(), m_vtxFormatTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       false, false, false);

    m_indTexZWrite = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_INDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                               ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, true, false);
    m_indTexNoZWrite = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_INDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                 ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 false, false, false);
    m_indTexAdditive = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_INDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                 ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 false, true, false);

    m_cindTexZWrite = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_CINDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                false, true, false);
    m_cindTexNoZWrite = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_CINDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                  ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                  false, false, false);
    m_cindTexAdditive = factory.newShaderPipeline(VS_HLSL_INDTEX, FS_HLSL_CINDTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                  ComPtr<ID3DBlob>(), m_vtxFormatIndTex,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  false, true, false);

    m_noTexZTestZWrite = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                   ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   true, true, false);
    m_noTexNoZTestZWrite = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     false, true, false);
    m_noTexZTestNoZWrite = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     true, false, false);
    m_noTexNoZTestNoZWrite = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                       ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       false, false, false);

    m_noTexAdditiveZTest = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                     ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     true, false, false);
    m_noTexAdditiveNoZTest = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                       ComPtr<ID3DBlob>(), m_vtxFormatNoTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                       false, false, false);

    return new struct D3DElementDataBindingFactory;
}

}
