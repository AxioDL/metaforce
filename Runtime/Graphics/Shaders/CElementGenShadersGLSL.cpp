#include "CElementGenShaders.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include "Particle/CSwooshDescription.hpp"
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
"SBINDING(0) out VertToFrag vtf;\n"
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
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(tex, vtf.uv);\n"
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
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"    colorOut.a = texture(tex, vtf.uv).r;\n"
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
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uvScene = uvsInScene;\n"
"    vtf.uvTexr = uvsInTexrTind[gl_VertexID].xy;\n"
"    vtf.uvTind = uvsInTexrTind[gl_VertexID].zw;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"    gl_Position = FLIPFROMGL(gl_Position);\n"
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
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texrMap;\n"
"TBINDING1 uniform sampler2D sceneMap;\n"
"TBINDING2 uniform sampler2D tindMap;\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(tindMap, vtf.uvTind).zw;\n"
"    vec4 sceneTexel = texture(sceneMap, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    vec4 texrTexel = texture(texrMap, vtf.uvTexr);\n"
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
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D texrMap;\n"
"TBINDING1 uniform sampler2D sceneMap;\n"
"TBINDING2 uniform sampler2D tindMap;\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(tindMap, vtf.uvTind).zw;\n"
"    vec4 sceneTexel = texture(sceneMap, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));\n"
"    colorOut = vtf.color * sceneTexel * texture(texrMap, vtf.uvTexr);\n"
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
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_NOTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

struct OGLElementDataBindingFactory : TShader<CElementGenShaders>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CElementGenShaders& shaders)
    {
        CElementGen& gen = shaders.m_gen;
        CGenDescription* desc = gen.GetDesc();

        boo::ObjToken<boo::IVertexFormat> vtxFmt;
        boo::ObjToken<boo::IVertexFormat> vtxFmtPmus;
        CUVElement* texr = desc->x54_x40_TEXR.get();
        CUVElement* tind = desc->x58_x44_TIND.get();
        int texCount = 0;
        boo::ObjToken<boo::ITexture> textures[3];

        if (texr)
        {
            textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
            texCount = 1;
            if (gen.m_instBuf)
            {
                if (tind)
                {
                    textures[1] = CGraphics::g_SpareTexture.get();
                    textures[2] = tind->GetValueTexture(0).GetObj()->GetBooTexture();
                    texCount = 3;

                    const boo::VertexElementDescriptor TexFmtIndTex[] =
                    {
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 4},
                    };
                    vtxFmt = ctx.newVertexFormat(10, TexFmtIndTex);
                }
                else
                {
                    const boo::VertexElementDescriptor TexFmtTex[] =
                    {
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                        {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
                    };
                    vtxFmt = ctx.newVertexFormat(9, TexFmtTex);
                }
            }
            if (gen.m_instBufPmus)
            {
                const boo::VertexElementDescriptor TexFmtTex[] =
                {
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
                };
                vtxFmtPmus = ctx.newVertexFormat(9, TexFmtTex);
            }
        }
        else
        {
            if (gen.m_instBuf)
            {
                const boo::VertexElementDescriptor TexFmtNoTex[] =
                {
                    {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf.get(), nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
                };
                vtxFmt = ctx.newVertexFormat(5, TexFmtNoTex);
            }
            if (gen.m_instBufPmus)
            {
                const boo::VertexElementDescriptor TexFmtNoTex[] =
                {
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBufPmus.get(), nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
                };
                vtxFmtPmus = ctx.newVertexFormat(5, TexFmtNoTex);
            }
        }

        if (gen.m_instBuf)
        {
            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBuf.get()};

            if (shaders.m_regPipeline)
                gen.m_normalDataBind = ctx.newShaderDataBinding(shaders.m_regPipeline, vtxFmt, nullptr,
                                                                gen.m_instBuf.get(), nullptr, 1, uniforms,
                                                                nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_regPipelineSub)
                gen.m_normalSubDataBind = ctx.newShaderDataBinding(shaders.m_regPipelineSub, vtxFmt, nullptr,
                                                                   gen.m_instBuf.get(), nullptr, 1, uniforms,
                                                                   nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipeline)
                gen.m_redToAlphaDataBind = ctx.newShaderDataBinding(shaders.m_redToAlphaPipeline, vtxFmt, nullptr,
                                                                    gen.m_instBuf.get(), nullptr, 1, uniforms,
                                                                    nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipelineSub)
                gen.m_redToAlphaSubDataBind = ctx.newShaderDataBinding(shaders.m_redToAlphaPipelineSub, vtxFmt, nullptr,
                                                                       gen.m_instBuf.get(), nullptr, 1, uniforms,
                                                                       nullptr, texCount, textures, nullptr, nullptr);
        }

        if (gen.m_instBufPmus)
        {
            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBufPmus.get()};
            texCount = std::min(texCount, 1);

            if (shaders.m_regPipelinePmus)
                gen.m_normalDataBindPmus = ctx.newShaderDataBinding(shaders.m_regPipelinePmus, vtxFmtPmus, nullptr,
                                                                    gen.m_instBufPmus.get(), nullptr, 1, uniforms,
                                                                    nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipelinePmus)
                gen.m_redToAlphaDataBindPmus = ctx.newShaderDataBinding(shaders.m_redToAlphaPipelinePmus, vtxFmtPmus, nullptr,
                                                                        gen.m_instBufPmus.get(), nullptr, 1, uniforms,
                                                                        nullptr, texCount, textures, nullptr, nullptr);
        }

        return nullptr;
    }
};

static const char* UniNames[] = {"ParticleUniform"};
static const char* TexNames[] = {"tex"};
static const char* TindTexNames[] = {"texrMap", "sceneMap", "tindMap"};

TShader<CElementGenShaders>::IDataBindingFactory* CElementGenShaders::Initialize(boo::GLDataFactory::Context& ctx)
{
    m_texZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                             true, false, boo::CullMode::None);
    m_texNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::None, true,
                                               true, false, boo::CullMode::None);
    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                 true, false, boo::CullMode::None);

    m_texAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                 true, false, boo::CullMode::None);

    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, TexNames, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_texRedToAlphaNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, TexNames, 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    m_texZTestNoZWriteSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                                  boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                  true, false, boo::CullMode::None);
    m_texNoZTestNoZWriteSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                                    boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                    boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                    true, false, boo::CullMode::None);

    m_texRedToAlphaZTestSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, TexNames, 1, UniNames,
                                                    boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                    boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                    true, false, boo::CullMode::None);
    m_texRedToAlphaNoZTestSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, TexNames, 1, UniNames,
                                                      boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                      boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                      true, false, boo::CullMode::None);

    m_indTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, TindTexNames, 1, UniNames,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           boo::Primitive::TriStrips, boo::ZTest::None, true,
                                           true, false, boo::CullMode::None);
    m_indTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, TindTexNames, 1, UniNames,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, boo::ZTest::None, false,
                                             true, false, boo::CullMode::None);
    m_indTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, TindTexNames, 1, UniNames,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                             boo::Primitive::TriStrips, boo::ZTest::None, true,
                                             true, false, boo::CullMode::None);

    m_cindTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, TindTexNames, 1, UniNames,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                            boo::Primitive::TriStrips, boo::ZTest::None, true,
                                            true, false, boo::CullMode::None);
    m_cindTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, TindTexNames, 1, UniNames,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                              boo::Primitive::TriStrips, boo::ZTest::None, false,
                                              true, false, boo::CullMode::None);
    m_cindTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, TindTexNames, 1, UniNames,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips, boo::ZTest::None, true,
                                              true, false, boo::CullMode::None);

    m_noTexZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                               true, false, boo::CullMode::None);
    m_noTexNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, true,
                                                 true, false, boo::CullMode::None);
    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_noTexNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_noTexAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    return new struct OGLElementDataBindingFactory;
}

template <>
void CElementGenShaders::Shutdown<boo::GLDataFactory>()
{
    m_texZTestZWrite.reset();
    m_texNoZTestZWrite.reset();
    m_texZTestNoZWrite.reset();
    m_texNoZTestNoZWrite.reset();
    m_texAdditiveZTest.reset();
    m_texAdditiveNoZTest.reset();
    m_texRedToAlphaZTest.reset();
    m_texRedToAlphaNoZTest.reset();
    m_texZTestNoZWriteSub.reset();
    m_texNoZTestNoZWriteSub.reset();
    m_texRedToAlphaZTestSub.reset();
    m_texRedToAlphaNoZTestSub.reset();

    m_indTexZWrite.reset();
    m_indTexNoZWrite.reset();
    m_indTexAdditive.reset();

    m_cindTexZWrite.reset();
    m_cindTexNoZWrite.reset();
    m_cindTexAdditive.reset();

    m_noTexZTestZWrite.reset();
    m_noTexNoZTestZWrite.reset();
    m_noTexZTestNoZWrite.reset();
    m_noTexNoZTestNoZWrite.reset();
    m_noTexAdditiveZTest.reset();
    m_noTexAdditiveNoZTest.reset();
}

#if BOO_HAS_VULKAN
struct VulkanElementDataBindingFactory : TShader<CElementGenShaders>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CElementGenShaders& shaders)
    {
        CElementGen& gen = shaders.m_gen;
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

        if (gen.m_instBuf)
        {
            boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBuf};

            if (shaders.m_regPipeline)
                gen.m_normalDataBind = ctx.newShaderDataBinding(shaders.m_regPipeline, nullptr, nullptr,
                                                                gen.m_instBuf, nullptr, 1, uniforms,
                                                                nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_regPipelineSub)
                gen.m_normalSubDataBind = ctx.newShaderDataBinding(shaders.m_regPipelineSub, nullptr, nullptr,
                                                                   gen.m_instBuf, nullptr, 1, uniforms,
                                                                   nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipeline)
                gen.m_redToAlphaDataBind = ctx.newShaderDataBinding(shaders.m_redToAlphaPipeline, nullptr, nullptr,
                                                                    gen.m_instBuf, nullptr, 1, uniforms,
                                                                    nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipelineSub)
                gen.m_redToAlphaSubDataBind = ctx.newShaderDataBinding(shaders.m_redToAlphaPipelineSub, nullptr, nullptr,
                                                                       gen.m_instBuf, nullptr, 1, uniforms,
                                                                       nullptr, texCount, textures, nullptr, nullptr);

        }

        if (gen.m_instBufPmus)
        {
            boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBufPmus};
            texCount = std::min(texCount, 1);

            if (shaders.m_regPipelinePmus)
                gen.m_normalDataBindPmus = ctx.newShaderDataBinding(shaders.m_regPipelinePmus, nullptr, nullptr,
                                                                    gen.m_instBufPmus, nullptr, 1, uniforms,
                                                                    nullptr, texCount, textures, nullptr, nullptr);
            if (shaders.m_redToAlphaPipelinePmus)
                gen.m_redToAlphaDataBindPmus = ctx.newShaderDataBinding(shaders.m_redToAlphaPipelinePmus, nullptr, nullptr,
                                                                        gen.m_instBufPmus, nullptr, 1, uniforms,
                                                                        nullptr, texCount, textures, nullptr, nullptr);
        }

        return nullptr;
    }
};

TShader<CElementGenShaders>::IDataBindingFactory* CElementGenShaders::Initialize(boo::VulkanDataFactory::Context& ctx)
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
                                             boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                             true, false, boo::CullMode::None);
    m_texNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::None, true,
                                               true, false, boo::CullMode::None);
    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                 true, false, boo::CullMode::None);

    m_texAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                               true, false, boo::CullMode::None);
    m_texAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                 true, false, boo::CullMode::None);

    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_texRedToAlphaNoZTest = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    m_texZTestNoZWriteSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                  boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                  boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                  true, false, boo::CullMode::None);
    m_texNoZTestNoZWriteSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_vtxFormatTex,
                                                    boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                    boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                    true, false, boo::CullMode::None);

    m_texRedToAlphaZTestSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                    boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                    boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                    true, false, boo::CullMode::None);
    m_texRedToAlphaNoZTestSub = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, m_vtxFormatTex,
                                                      boo::BlendFactor::Subtract, boo::BlendFactor::Subtract,
                                                      boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                      true, false, boo::CullMode::None);

    m_indTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                           true, false, boo::CullMode::None);
    m_indTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                             true, false, boo::CullMode::None);
    m_indTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, m_vtxFormatIndTex,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                             boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                             true, false, boo::CullMode::None);

    m_cindTexZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                            boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                            true, false, boo::CullMode::None);
    m_cindTexNoZWrite = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                              boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                              true, false, boo::CullMode::None);
    m_cindTexAdditive = ctx.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, m_vtxFormatIndTex,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                              true, false, boo::CullMode::None);

    m_noTexZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, true,
                                               true, false, boo::CullMode::None);
    m_noTexNoZTestZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::None, true,
                                                 true, false, boo::CullMode::None);
    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_noTexNoZTestNoZWrite = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false,
                                                 true, false, boo::CullMode::None);
    m_noTexAdditiveNoZTest = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_vtxFormatNoTex,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   boo::Primitive::TriStrips, boo::ZTest::None, false,
                                                   true, false, boo::CullMode::None);

    return new struct VulkanElementDataBindingFactory;
}
#endif

}
