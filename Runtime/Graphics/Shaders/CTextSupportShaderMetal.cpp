#include "CTextSupportShader.hpp"
#include "GuiSys/CTextRenderBuffer.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* TextVS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct InstData\n"
"{\n"
"    float4 posIn[4];\n"
"    float4 uvIn[4];\n"
"    float4 fontColorIn;\n"
"    float4 outlineColorIn;\n"
"};\n"
"\n"
"struct TextSupportUniform\n"
"{\n"
"    float4x4 mtx;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 fontColor;\n"
"    float4 outlineColor;\n"
"    float3 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(constant InstData* instArr [[ buffer(1) ]],\n"
"                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
"                        constant TextSupportUniform& uData [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant InstData& inst = instArr[instId];\n"
"    vtf.fontColor = uData.color * inst.fontColorIn;\n"
"    vtf.outlineColor = uData.color * inst.outlineColorIn;\n"
"    vtf.uv = inst.uvIn[vertId].xyz;\n"
"    vtf.pos = uData.mtx * float4(inst.posIn[vertId].xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* TextFS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat);\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 fontColor;\n"
"    float4 outlineColor;\n"
"    float3 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d_array<float> tex [[ texture(0) ]])\n"
"{\n"
"    float4 texel = tex.sample(samp, vtf.uv.xy, vtf.uv.z);\n"
"    return vtf.fontColor * texel.r + vtf.outlineColor * texel.g;\n"
"}\n";

static const char* ImgVS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct InstData\n"
"{\n"
"    float4 posIn[4];\n"
"    float4 uvIn[4];\n"
"    float4 colorIn;\n"
"};\n"
"\n"
"struct TextSupportUniform\n"
"{\n"
"    float4x4 mtx;\n"
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
"vertex VertToFrag vmain(constant InstData* instArr [[ buffer(1) ]],\n"
"                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
"                        constant TextSupportUniform& uData [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant InstData& inst = instArr[instId];\n"
"    vtf.color = uData.color * inst.colorIn;\n"
"    vtf.uv = inst.uvIn[vertId].xy;\n"
"    vtf.pos = uData.mtx * float4(inst.posIn[vertId].xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* ImgFS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat);\n"
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
"    float4 texel = tex.sample(samp, vtf.uv);\n"
"    return vtf.color * texel;\n"
"}\n";

TMultiBlendShader<CTextSupportShader>::IDataBindingFactory*
CTextSupportShader::Initialize(boo::MetalDataFactory::Context& ctx)
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
    };
    s_TextVtxFmt = ctx.newVertexFormat(10, TextVtxVmt);
    s_TextAlphaPipeline = ctx.newShaderPipeline(TextVS, TextFS, s_TextVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                                boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    s_TextAddPipeline = ctx.newShaderPipeline(TextVS, TextFS, s_TextVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                              boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);

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
    s_ImageAlphaPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, s_ImageVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                                 boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    s_ImageAddPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, s_ImageVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                               boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);

    return nullptr;
}

}
