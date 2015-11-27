#include "Specter/View.hpp"
#include "Specter/ViewSystem.hpp"

namespace Specter
{

void View::System::init(boo::GLDataFactory* factory)
{
    static const char* VS =
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

    static const char* FS =
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

    static const char* BlockNames[] = {"SpecterViewBlock"};

    m_solidShader = factory->newShaderPipeline(VS, FS, 0, nullptr, 1, BlockNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);
}

void View::System::init(boo::ID3DDataFactory* factory)
{
    static const char* VS =
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

    static const char* FS =
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "float4 main(in VertToFrag vtf) : SV_Target0\n"
    "{\n"
    "    return vtf.color;\n"
    "}\n";

    boo::VertexElementDescriptor vdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFmt = factory->newVertexFormat(2, vdescs);

    ComPtr<ID3DBlob> vertBlob;
    ComPtr<ID3DBlob> fragBlob;
    ComPtr<ID3DBlob> pipeBlob;
    m_solidShader = factory->newShaderPipeline(VS, FS, vertBlob, fragBlob, pipeBlob, m_vtxFmt,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);
}

View::View(ViewSystem& system)
{
    m_bgVertBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CVector3f), 4);

    m_bgInstBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CColor), 1);

    m_viewVertBlockBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform,
                                       sizeof(VertexBlock), 1);

    if (!system.m_viewSystem.m_vtxFmt)
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
        system.m_factory->newShaderDataBinding(system.m_viewSystem.m_solidShader, system.m_viewSystem.m_vtxFmt,
                                               m_bgVertBuf, m_bgInstBuf, nullptr, 1,
                                               (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                               0, nullptr);
    }

}

void View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
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

}
