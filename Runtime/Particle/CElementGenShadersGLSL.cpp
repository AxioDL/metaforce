#include "CElementGenShaders.hpp"
#include "CElementGen.hpp"
#include "CGenDescription.hpp"
#include "CElectricDescription.hpp"
#include "CSwooshDescription.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{

static const char* VS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"layout(location=5) in vec4 uvsIn[4];\n"
"\n"
"UBINDING0 uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uv = uvsIn[gl_VertexID].xy;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texs[1];\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(texs[0], vtf.uv);\n"
"}\n";

static const char* FS_GLSL_TEX_REDTOALPHA =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texs[1];\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(texs[0], vtf.uv);\n"
"    colorOut.a = colorOut.r;\n"
"}\n";

static const char* VS_GLSL_INDTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"layout(location=5) in vec4 uvsInTexrTind[4];\n"
"layout(location=9) in vec4 uvsInScene;\n"
"\n"
"UBINDING0 uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 uvScene;\n"
"    vec2 uvTexr;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uvScene = uvsInScene;\n"
"    vtf.uvTexr = uvsInTexrTind[gl_VertexID].xy;\n"
"    vtf.uvTind = uvsInTexrTind[gl_VertexID].zw;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_INDTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 uvScene;\n"
"    vec2 uvTexr;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texs[3];\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(texs[2], vtf.uvTind).zw;\n"
"    vec4 sceneTexel = texture(texs[1], mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    vec4 texrTexel = texture(texs[0], vtf.uvTexr);\n"
"    colorOut = vtf.color * sceneTexel + texrTexel;\n"
"    colorOut.a = vtf.color.a * texrTexel.a;\n"
"}\n";

static const char* FS_GLSL_CINDTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 uvScene;\n"
"    vec2 uvTexr;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texs[3];\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(texs[2], vtf.uvTind).zw;\n"
"    vec4 sceneTexel = texture(texs[1], mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    colorOut = vtf.color * sceneTexel * texture(texs[0], vtf.uvTexr);\n"
"}\n";

static const char* VS_GLSL_NOTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"\n"
"UBINDING0 uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_NOTEX =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

struct OGLElementDataBindingFactory : CElementGenShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                CElementGen& gen,
                                boo::IShaderPipeline* regPipeline,
                                boo::IShaderPipeline* redToAlphaPipeline)
    {
        CGenDescription* desc = gen.GetDesc();

        boo::IVertexFormat* vtxFmt = nullptr;
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

                const boo::VertexElementDescriptor TexFmtIndTex[] =
                {
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 4},
                };
                vtxFmt = ctx.newVertexFormat(10, TexFmtIndTex);
            }
            else
            {
                const boo::VertexElementDescriptor TexFmtTex[] =
                {
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
                };
                vtxFmt = ctx.newVertexFormat(9, TexFmtTex);
            }
        }
        else
        {
            const boo::VertexElementDescriptor TexFmtNoTex[] =
            {
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
            };
            vtxFmt = ctx.newVertexFormat(5, TexFmtNoTex);
        }

        boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBuf};

        if (regPipeline)
            gen.m_normalDataBind = ctx.newShaderDataBinding(regPipeline, vtxFmt, nullptr,
                                                            gen.m_instBuf, nullptr, 1, uniforms,
                                                            texCount, textures);
        if (redToAlphaPipeline)
            gen.m_redToAlphaDataBind = ctx.newShaderDataBinding(redToAlphaPipeline, vtxFmt, nullptr,
                                                                gen.m_instBuf, nullptr, 1, uniforms,
                                                                texCount, textures);
    }
};

CElementGenShaders::IDataBindingFactory* CElementGenShaders::Initialize(boo::GLDataFactory::Context& ctx)
{
    static const char* UniNames[] = {"ParticleUniform"};

    m_texZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, true, true, false);
    m_texNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, false, true, false);
    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, true, false, false);
    m_texNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, false, false, false);

    m_texAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   boo::Primitive::TriStrips, true, false, false);
    m_texAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     boo::Primitive::TriStrips, false, false, false);

    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_texRedToAlphaNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, "texs", 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       boo::Primitive::TriStrips, false, false, false);

    m_indTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, false, true, false);
    m_indTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, false, false, false);
    m_indTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, false, true, false);

    m_cindTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                boo::Primitive::TriStrips, false, true, false);
    m_cindTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                  boo::Primitive::TriStrips, false, false, false);
    m_cindTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, false, true, false);

    m_noTexZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, true, true, false);
    m_noTexNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, false, true, false);
    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_noTexNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       boo::Primitive::TriStrips, false, false, false);

    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_noTexAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                       boo::Primitive::TriStrips, false, false, false);

    return new struct OGLElementDataBindingFactory;
}

#if BOO_HAS_VULKAN
struct VulkanElementDataBindingFactory : CElementGenShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                CElementGen& gen,
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
            gen.m_normalDataBind = ctx.newShaderDataBinding(regPipeline, nullptr, nullptr,
                                                            gen.m_instBuf, nullptr, 1, uniforms,
                                                            texCount, textures);
        if (redToAlphaPipeline)
            gen.m_redToAlphaDataBind = ctx.newShaderDataBinding(redToAlphaPipeline, nullptr, nullptr,
                                                                gen.m_instBuf, nullptr, 1, uniforms,
                                                                texCount, textures);
    }
};

CElementGenShaders::IDataBindingFactory* CElementGenShaders::Initialize(boo::VulkanDataFactory::Context& ctx)
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
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 4},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 5},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 6},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 7}
    };
    m_vtxFormatIndTex = ctx.newVertexFormat(13, TexFmtIndTex);

    static const boo::VertexElementDescriptor TexFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFormatNoTex = ctx.newVertexFormat(5, TexFmtNoTex);

    m_texZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, true, true, false);
    m_texNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, false, true, false);
    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, true, false, false);
    m_texNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, false, false, false);

    m_texAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   boo::Primitive::TriStrips, true, false, false);
    m_texAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     boo::Primitive::TriStrips, false, false, false);

    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_texRedToAlphaNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       boo::Primitive::TriStrips, false, false, false);

    m_indTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, true, true, false);
    m_indTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, true, false, false);
    m_indTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, true, true, false);

    m_cindTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                boo::Primitive::TriStrips, true, true, false);
    m_cindTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                  boo::Primitive::TriStrips, true, false, false);
    m_cindTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  boo::Primitive::TriStrips, true, true, false);

    m_noTexZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, true, true, false);
    m_noTexNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, false, true, false);
    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_noTexNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       boo::Primitive::TriStrips, false, false, false);

    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     boo::Primitive::TriStrips, true, false, false);
    m_noTexAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                       boo::Primitive::TriStrips, false, false, false);

    return new struct VulkanElementDataBindingFactory;
}
#endif

}
