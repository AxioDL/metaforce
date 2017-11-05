#include "CTextSupportShader.hpp"
#include "GuiSys/CTextRenderBuffer.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* TextVS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 uvIn[4];\n"
"layout(location=8) in vec4 fontColorIn;\n"
"layout(location=9) in vec4 outlineColorIn;\n"
"layout(location=10) in vec4 mulColorIn;\n"
"\n"
"UBINDING0 uniform TextSupportUniform\n"
"{\n"
"    mat4 mtx;\n"
"    vec4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 fontColor;\n"
"    vec4 outlineColor;\n"
"    vec4 mulColor;\n"
"    vec3 uv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.fontColor = color * fontColorIn;\n"
"    vtf.outlineColor = color * outlineColorIn;\n"
"    vtf.mulColor = mulColorIn;\n"
"    vtf.uv = uvIn[gl_VertexID].xyz;\n"
"    gl_Position = mtx * vec4(posIn[gl_VertexID].xyz, 1.0);\n"
"}\n";

static const char* TextFS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 fontColor;\n"
"    vec4 outlineColor;\n"
"    vec4 mulColor;\n"
"    vec3 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2DArray tex;\n"
"void main()\n"
"{\n"
"    vec4 texel = texture(tex, vtf.uv);\n"
"    colorOut = (vtf.fontColor * texel.r + vtf.outlineColor * texel.g) * vtf.mulColor;\n"
"}\n";

static const char* ImgVS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec3 posIn[4];\n"
"layout(location=4) in vec2 uvIn[4];\n"
"layout(location=8) in vec4 colorIn;\n"
"\n"
"UBINDING0 uniform TextSupportUniform\n"
"{\n"
"    mat4 mtx;\n"
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
"    vtf.color = color * colorIn;\n"
"    vtf.uv = uvIn[gl_VertexID];\n"
"    gl_Position = mtx * vec4(posIn[gl_VertexID].xyz, 1.0);\n"
"}\n";

static const char* ImgFS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    vec4 texel = texture(tex, vtf.uv);\n"
"    colorOut = vtf.color * texel;\n"
"}\n";

TMultiBlendShader<CTextSupportShader>::IDataBindingFactory*
CTextSupportShader::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TextSupportUniform"};

    s_TextAlphaPipeline = ctx.newShaderPipeline(TextVS, TextFS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                                boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_TextAddPipeline = ctx.newShaderPipeline(TextVS, TextFS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                              boo::BlendFactor::One, boo::Primitive::TriStrips,
                                              boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    s_ImageAlphaPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                                 boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                 boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_ImageAddPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                               boo::BlendFactor::One, boo::Primitive::TriStrips,
                                               boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    return nullptr;
}

template <>
void CTextSupportShader::Shutdown<boo::GLDataFactory>()
{
    s_TextAlphaPipeline.reset();
    s_TextAddPipeline.reset();
    s_ImageAlphaPipeline.reset();
    s_ImageAddPipeline.reset();
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CTextSupportShader>::IDataBindingFactory*
CTextSupportShader::Initialize(boo::VulkanDataFactory::Context& ctx)
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
    s_TextAlphaPipeline = ctx.newShaderPipeline(TextVS, TextFS, s_TextVtxFmt, boo::BlendFactor::SrcAlpha,
                                                boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_TextAddPipeline = ctx.newShaderPipeline(TextVS, TextFS, s_TextVtxFmt, boo::BlendFactor::SrcAlpha,
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
    s_ImageAlphaPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, s_ImageVtxFmt, boo::BlendFactor::SrcAlpha,
                                                 boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                 boo::ZTest::LEqual, false, true, false, boo::CullMode::None);
    s_ImageAddPipeline = ctx.newShaderPipeline(ImgVS, ImgFS, s_ImageVtxFmt, boo::BlendFactor::SrcAlpha,
                                               boo::BlendFactor::One, boo::Primitive::TriStrips,
                                               boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    return nullptr;
}
#endif

}
