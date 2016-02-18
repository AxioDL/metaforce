#include "CLineRendererShaders.hpp"
#include "CLineRenderer.hpp"

namespace pshag
{

static const char* VS_HLSL_TEX =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer LineUniform : register(b0)\n"
"{\n"
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
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn * moduColor;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = v.posIn;\n"
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

static const char* VS_HLSL_NOTEX =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer LineUniform : register(b0)\n"
"{\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn * moduColor;\n"
"    vtf.position = v.posIn;\n"
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

struct HLSLLineDataBindingFactory : CLineRendererShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(CLineRenderer& renderer, boo::IShaderPipeline* pipeline, boo::ITexture* texture)
    {
        int texCount = 0;
        boo::ITexture* textures[1];

        if (texture)
        {
            textures[0] = texture;
            texCount = 1;
        }

        boo::IGraphicsBuffer* uniforms[] = {renderer.m_uniformBuf};

        renderer.m_shaderBind = CGraphics::g_BooFactory->newShaderDataBinding(pipeline, nullptr, renderer.m_vertBuf,
                                                                              nullptr, nullptr, 1, uniforms,
                                                                              texCount, textures);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::ID3DDataFactory& factory)
{
    static const boo::VertexElementDescriptor VtxFmtTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    m_texVtxFmt = factory.newVertexFormat(3, VtxFmtTex);

    static const boo::VertexElementDescriptor VtxFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color}
    };
    m_noTexVtxFmt = factory.newVertexFormat(2, VtxFmtNoTex);

    m_texAlpha = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                           ComPtr<ID3DBlob>(), m_texVtxFmt,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           false, true, false);
    m_texAdditive = factory.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                              ComPtr<ID3DBlob>(), m_texVtxFmt,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              false, false, false);
    m_noTexAlpha = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                             ComPtr<ID3DBlob>(), m_noTexVtxFmt,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             false, true, false);
    m_noTexAdditive = factory.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                ComPtr<ID3DBlob>(), m_noTexVtxFmt,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                false, false, false);

    return new struct HLSLLineDataBindingFactory;
}

}
