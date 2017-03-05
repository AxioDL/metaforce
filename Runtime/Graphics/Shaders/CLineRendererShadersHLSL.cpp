#include "CLineRendererShaders.hpp"
#include "Graphics/CLineRenderer.hpp"

namespace urde
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
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                boo::IShaderPipeline* pipeline, boo::ITexture* texture)
    {
        int texCount = 0;
        boo::ITexture* textures[1];

        if (texture)
        {
            textures[0] = texture;
            texCount = 1;
        }

        boo::IGraphicsBuffer* uniforms[] = {renderer.m_uniformBuf};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, nullptr, renderer.m_vertBuf,
                                                         nullptr, nullptr, 1, uniforms, nullptr,
                                                         texCount, textures);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor VtxFmtTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    m_texVtxFmt = ctx.newVertexFormat(3, VtxFmtTex);

    static const boo::VertexElementDescriptor VtxFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color}
    };
    m_noTexVtxFmt = ctx.newVertexFormat(2, VtxFmtNoTex);

    m_texAlpha = ctx.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, nullptr, nullptr,
                                           nullptr, m_texVtxFmt,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           boo::Primitive::TriStrips,false, true, false);
    m_texAdditive = ctx.newShaderPipeline(VS_HLSL_TEX, FS_HLSL_TEX, nullptr, nullptr,
                                              nullptr, m_texVtxFmt,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips,false, false, false);
    m_noTexAlpha = ctx.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, nullptr, nullptr,
                                             nullptr, m_noTexVtxFmt,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, false, true, false);
    m_noTexAdditive = ctx.newShaderPipeline(VS_HLSL_NOTEX, FS_HLSL_NOTEX, nullptr, nullptr,
                                                nullptr, m_noTexVtxFmt,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, false, false, false);

    return new struct HLSLLineDataBindingFactory;
}

}
