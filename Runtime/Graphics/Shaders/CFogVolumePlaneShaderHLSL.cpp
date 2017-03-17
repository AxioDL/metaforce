#include "CFogVolumePlaneShader.hpp"
#include "TShader.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"};\n"
"\n"
"float4 main(in VertData v) : SV_Position\n"
"{\n"
"    return v.posIn;\n"
"}\n";

static const char* FS =
"float4 main() : SV_Target0\n"
"{\n"
"    return float4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumePlaneShader)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipelines[4] = {};

struct CFogVolumePlaneShaderD3DDataBindingFactory : TShader<CFogVolumePlaneShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CFogVolumePlaneShader& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        for (int i=0 ; i<4 ; ++i)
            filter.m_dataBinds[i] = cctx.newShaderDataBinding(s_Pipelines[i], s_VtxFmt,
                filter.m_vbo, nullptr, nullptr, 0, nullptr,
                nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBinds[0];
    }
};

TShader<CFogVolumePlaneShader>::IDataBindingFactory*
CFogVolumePlaneShader::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipelines[0] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Frontface);
    s_Pipelines[1] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, false, boo::CullMode::Frontface);
    s_Pipelines[2] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Backface);
    s_Pipelines[3] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::Greater, false, false, false, boo::CullMode::Backface);
    return new CFogVolumePlaneShaderD3DDataBindingFactory;
}

}
