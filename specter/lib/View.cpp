#include "Specter/View.hpp"
#include "Specter/ViewSystem.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{

void View::System::init(boo::GLDataFactory* factory)
{
    static const char* SolidVS =
    "#version 330\n"
    "layout(location=0) in vec3 posIn;\n"
    "layout(location=1) in vec4 colorIn;\n"
    SPECTER_VIEW_VERT_BLOCK_GLSL
    "struct VertToFrag\n"
    "{\n"
    "    vec4 color;\n"
    "};\n"
    "out VertToFrag vtf;\n"
    "void main()\n"
    "{\n"
    "    vtf.color = colorIn;\n"
    "    gl_Position = mv * vec4(posIn, 1.0);\n"
    "}\n";

    static const char* SolidFS =
    "#version 330\n"
    "struct VertToFrag\n"
    "{\n"
    "    vec4 color;\n"
    "};\n"
    "in VertToFrag vtf;\n"
    "layout(location=0) out vec4 colorOut;\n"
    "void main()\n"
    "{\n"
    "    colorOut = vtf.color;\n"
    "}\n";

    static const char* TexVS =
    "#version 330\n"
    "layout(location=0) in vec3 posIn;\n"
    "layout(location=1) in vec2 uvIn;\n"
    SPECTER_VIEW_VERT_BLOCK_GLSL
    "struct VertToFrag\n"
    "{\n"
    "    vec2 uv;\n"
    "};\n"
    "out VertToFrag vtf;\n"
    "void main()\n"
    "{\n"
    "    vtf.uv = uvIn;\n"
    "    gl_Position = mv * vec4(posIn, 1.0);\n"
    "}\n";

    static const char* TexFS =
    "#version 330\n"
    "struct VertToFrag\n"
    "{\n"
    "    vec2 uv;\n"
    "};\n"
    "in VertToFrag vtf;\n"
    "uniform sampler2D tex;\n"
    "layout(location=0) out vec4 colorOut;\n"
    "void main()\n"
    "{\n"
    "    colorOut = texture(tex, vtf.uv);\n"
    "}\n";

    static const char* BlockNames[] = {"SpecterViewBlock"};

    m_solidShader = factory->newShaderPipeline(SolidVS, SolidFS, 0, nullptr, 1, BlockNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);

    m_texShader = factory->newShaderPipeline(TexVS, TexFS, 1, "tex", 1, BlockNames,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             false, false, false);
}
    
#if _WIN32

void View::System::init(boo::ID3DDataFactory* factory)
{
    static const char* SolidVS =
    "struct VertData\n"
    "{\n"
    "    float3 posIn : POSITION;\n"
    "    float4 colorIn : COLOR;\n"
    "};\n"
    SPECTER_VIEW_VERT_BLOCK_HLSL
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "VertToFrag main(in VertData v)\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    vtf.color = v.colorIn;\n"
    "    vtf.position = mul(mv, float4(v.posIn, 1.0));\n"
    "    return vtf;\n"
    "}\n";

    static const char* SolidFS =
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "float4 main(in VertToFrag vtf) : SV_Target0\n"
    "{\n"
    "    return vtf.color;\n"
    "}\n";

    static const char* TexVS =
    "struct VertData\n"
    "{\n"
    "    float3 posIn : POSITION;\n"
    "    float2 uvIn : UV;\n"
    "};\n"
    SPECTER_VIEW_VERT_BLOCK_HLSL
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float2 uv : UV;\n"
    "};\n"
    "VertToFrag main(in VertData v)\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    vtf.uv = v.uvIn;\n"
    "    vtf.position = mul(mv, float4(v.posIn, 1.0));\n"
    "    return vtf;\n"
    "}\n";

    static const char* TexFS =
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float2 uv : UV;\n"
    "};\n"
    "Texture2D tex : register(t0);\n"
    "SamplerState samp : register(s0);\n"
    "float4 main(in VertToFrag vtf) : SV_Target0\n"
    "{\n"
    "    return tex.Sample(samp, vtf.uv);\n"
    "}\n";

    boo::VertexElementDescriptor solidvdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_solidVtxFmt = factory->newVertexFormat(2, solidvdescs);

    ComPtr<ID3DBlob> vertBlob;
    ComPtr<ID3DBlob> fragBlob;
    ComPtr<ID3DBlob> pipeBlob;
    m_solidShader = factory->newShaderPipeline(SolidVS, SolidFS, vertBlob, fragBlob, pipeBlob, m_solidVtxFmt,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);

    boo::VertexElementDescriptor texvdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    m_texVtxFmt = factory->newVertexFormat(2, texvdescs);

    vertBlob.Reset();
    fragBlob.Reset();
    pipeBlob.Reset();
    m_texShader = factory->newShaderPipeline(TexVS, TexFS, vertBlob, fragBlob, pipeBlob, m_texVtxFmt,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             false, false, false);
}
    
#elif BOO_HAS_METAL
    
void View::System::init(boo::MetalDataFactory* factory)
{
    static const char* SolidVS =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertData\n"
    "{\n"
    "    float3 posIn [[ attribute(0) ]];\n"
    "    float4 colorIn [[ attribute(1) ]];\n"
    "};\n"
    SPECTER_VIEW_VERT_BLOCK_METAL
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float4 color;\n"
    "};\n"
    "vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpecterViewBlock& view [[ buffer(2) ]])\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    vtf.color = v.colorIn;\n"
    "    vtf.position = view.mv * float4(v.posIn, 1.0);\n"
    "    return vtf;\n"
    "}\n";
    
    static const char* SolidFS =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float4 color;\n"
    "};\n"
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
    "{\n"
    "    return vtf.color;\n"
    "}\n";

    static const char* TexVS =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertData\n"
    "{\n"
    "    float3 posIn [[ attribute(0) ]];\n"
    "    float2 uvIn [[ attribute(1) ]];\n"
    "};\n"
    SPECTER_VIEW_VERT_BLOCK_METAL
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float2 uv;\n"
    "};\n"
    "vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpecterViewBlock& view [[ buffer(2) ]])\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    vtf.uv = v.uvIn;\n"
    "    vtf.position = view.mv * float4(v.posIn, 1.0);\n"
    "    return vtf;\n"
    "}\n";

    static const char* TexFS =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "constexpr sampler samp(address::repeat);\n"
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float2 uv;\n"
    "};\n"
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
    "{\n"
    "    return tex.sample(samp, vtf.uv);\n"
    "}\n";
    
    boo::VertexElementDescriptor solidvdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_solidVtxFmt = factory->newVertexFormat(2, solidvdescs);
    
    m_solidShader = factory->newShaderPipeline(SolidVS, SolidFS, m_solidVtxFmt, 1,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);

    boo::VertexElementDescriptor texvdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    m_texVtxFmt = factory->newVertexFormat(2, texvdescs);

    m_texShader = factory->newShaderPipeline(TexVS, TexFS, m_texVtxFmt, 1,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             false, false, false);
}
    
#endif

void View::buildResources(ViewSystem& system)
{
    m_bgColor = Zeus::CColor::skClear;

    m_bgVertBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CVector3f), 4);

    m_bgInstBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CColor), 1);

    m_viewVertBlockBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform,
                                       sizeof(VertexBlock), 1);

    if (!system.m_viewSystem.m_solidVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_bgVertBuf, nullptr, boo::VertexSemantic::Position4},
            {m_bgInstBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
        };
        m_bgVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        m_bgShaderBinding =
        system.m_factory->newShaderDataBinding(system.m_viewSystem.m_solidShader, m_bgVtxFmt,
                                               m_bgVertBuf, m_bgInstBuf, nullptr, 1,
                                               (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                               0, nullptr);
    }
    else
    {
        m_bgShaderBinding =
        system.m_factory->newShaderDataBinding(system.m_viewSystem.m_solidShader, system.m_viewSystem.m_solidVtxFmt,
                                               m_bgVertBuf, m_bgInstBuf, nullptr, 1,
                                               (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                               0, nullptr);
    }
}

View::View(ViewSystem& system, RootView& rootView)
: m_rootView(rootView),
  m_parentView(rootView)
{
    buildResources(system);
}

View::View(ViewSystem& system, View& parentView)
: m_rootView(parentView.root()),
  m_parentView(parentView)
{
    buildResources(system);
}

void View::updateSize()
{
    resized(m_rootView.rootRect(), m_subRect);
}

void View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    m_subRect = sub;
    m_viewVertBlock.setViewRect(root, sub);
    m_bgRect[0].assign(0.f, sub.size[1], 0.f);
    m_bgRect[1].assign(0.f, 0.f, 0.f);
    m_bgRect[2].assign(sub.size[0], sub.size[1], 0.f);
    m_bgRect[3].assign(sub.size[0], 0.f, 0.f);
    m_bgValidSlots = 0;
}

void View::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    int pendingSlot = 1 << gfxQ->pendingDynamicSlot();
    if ((m_bgValidSlots & pendingSlot) == 0)
    {
        m_viewVertBlockBuf->load(&m_viewVertBlock, sizeof(VertexBlock));
        m_bgVertBuf->load(m_bgRect, sizeof(Zeus::CVector3f) * 4);
        m_bgInstBuf->load(&m_bgColor, sizeof(Zeus::CColor));
        m_bgValidSlots |= pendingSlot;
    }
    gfxQ->setShaderDataBinding(m_bgShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 4);
}

void View::commitResources(ViewSystem& system)
{
    m_gfxData.reset(system.m_factory->commit());
}

}
