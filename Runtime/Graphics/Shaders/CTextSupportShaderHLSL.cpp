#include "CTextSupportShader.hpp"
#include "GuiSys/CTextRenderBuffer.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* TextVS =
"struct InstData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 uvIn[4] : UV;\n"
"    float4 fontColorIn : COLOR0;\n"
"    float4 outlineColorIn : COLOR1;\n"
"    float4 mulColorIn : COLOR2;\n"
"};\n"
"\n"
"cbuffer TextSupportUniform : register(b0)\n"
"{\n"
"    float4x4 mtx;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 fontColor : COLOR0;\n"
"    float4 outlineColor : COLOR1;\n"
"    float4 mulColor : COLOR2;\n"
"    float3 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in InstData inst, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.fontColor = color * inst.fontColorIn;\n"
"    vtf.outlineColor = color * inst.outlineColorIn;\n"
"    vtf.mulColor = inst.mulColorIn;\n"
"    vtf.uv = inst.uvIn[vertId].xyz;\n"
"    vtf.pos = mul(mtx, float4(inst.posIn[vertId].xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* TextFS =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 fontColor : COLOR0;\n"
"    float4 outlineColor : COLOR1;\n"
"    float4 mulColor : COLOR2;\n"
"    float3 uv : UV;\n"
"};\n"
"\n"
"Texture2DArray tex : register(t0);\n"
"SamplerState samp : register(s2);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 texel = tex.Sample(samp, vtf.uv.xyz);\n"
"    return (vtf.fontColor * texel.r + vtf.outlineColor * texel.g) * vtf.mulColor;\n"
"}\n";

static const char* ImgVS =
"struct InstData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 uvIn[4] : UV;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer TextSupportUniform : register(b0)\n"
"{\n"
"    float4x4 mtx;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in InstData inst, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color * inst.colorIn;\n"
"    vtf.uv = inst.uvIn[vertId].xy;\n"
"    vtf.pos = mul(mtx, float4(inst.posIn[vertId].xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* ImgFS =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s2);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 texel = tex.Sample(samp, vtf.uv);\n"
"    return vtf.color * texel;\n"
"}\n";

TMultiBlendShader<CTextSupportShader>::IDataBindingFactory*
CTextSupportShader::Initialize(boo::D3DDataFactory::Context& ctx)
{
    boo::VertexElementDescriptor TextVtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 2},
    };
    s_TextVtxFmt = ctx.newVertexFormat(11, TextVtxVmt);
    s_TextAlphaPipeline = ctx.newShaderPipeline(TextVS, TextFS, nullptr, nullptr, nullptr,
                                                s_TextVtxFmt, boo::BlendFactor::SrcAlpha,
                                                boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_TextAddPipeline = ctx.newShaderPipeline(TextVS, TextFS, nullptr, nullptr, nullptr,
                                              s_TextVtxFmt, boo::BlendFactor::SrcAlpha,
                                              boo::BlendFactor::One, boo::Primitive::TriStrips,
                                              boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_TextAddOverdrawPipeline = ctx.newShaderPipeline(TextVS, TextFS, nullptr, nullptr, nullptr,
                                                s_TextVtxFmt, boo::BlendFactor::One,
                                                boo::BlendFactor::One, boo::Primitive::TriStrips,
                                                boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    boo::VertexElementDescriptor ImageVtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 0},
    };
    s_ImageVtxFmt = ctx.newVertexFormat(9, ImageVtxVmt);
    s_ImageAlphaPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, nullptr, nullptr, nullptr,
                                                 s_ImageVtxFmt, boo::BlendFactor::SrcAlpha,
                                                 boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                 boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_ImageAddPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, nullptr, nullptr, nullptr,
                                               s_ImageVtxFmt, boo::BlendFactor::SrcAlpha,
                                               boo::BlendFactor::One, boo::Primitive::TriStrips,
                                               boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_ImageAddOverdrawPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, nullptr, nullptr, nullptr,
                                                 s_ImageVtxFmt, boo::BlendFactor::One,
                                                 boo::BlendFactor::One, boo::Primitive::TriStrips,
                                                 boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    return nullptr;
}

template <>
void CTextSupportShader::Shutdown<boo::D3DDataFactory>()
{
    s_TextVtxFmt.reset();
    s_TextAlphaPipeline.reset();
    s_TextAddPipeline.reset();
    s_TextAddOverdrawPipeline.reset();
    s_ImageVtxFmt.reset();
    s_ImageAlphaPipeline.reset();
    s_ImageAddPipeline.reset();
    s_ImageAddOverdrawPipeline.reset();
}
}
