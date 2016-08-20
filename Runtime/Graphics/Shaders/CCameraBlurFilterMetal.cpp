#include "CCameraBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform ThermalHotUniform\n"
"{\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.sceneUv = uvIn.xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"TBINDING1 uniform sampler2D paletteTex;\n"
"const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);\n"
"void main()\n"
"{\n"
"    float sceneSample = dot(texture(sceneTex, vtf.sceneUv), kRGBToYPrime);\n"
"    vec4 colorSample = texture(paletteTex, vec2(sceneSample / 17.0, 0.5));\n"
"    colorOut = colorSample * sceneSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)

struct CCameraBlurFilterMetalDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CCameraBlurFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, g_Renderer->GetThermoPalette()};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::MetalDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut,
                                                                               boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::DstAlpha,
                                    boo::BlendFactor::InvDstAlpha, boo::Primitive::TriStrips, false, false, false);
    return new CCameraBlurFilterMetalDataBindingFactory;
}

}
