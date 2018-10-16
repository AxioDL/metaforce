#include "specter/View.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"
#include "hecl/Pipeline.hpp"

namespace specter
{
static logvisor::Module Log("specter::View");

zeus::CMatrix4f g_PlatformMatrix;

void View::Resources::init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme)
{
    switch (ctx.platform())
    {
    case boo::IGraphicsDataFactory::Platform::Vulkan:
        g_PlatformMatrix.m[1][1] = -1.f;
        break;
    default:
        break;
    }
    m_solidShader = hecl::conv->convert(ctx, Shader_SpecterViewShaderSolid{});
    m_texShader = hecl::conv->convert(ctx, Shader_SpecterViewShaderTex{});
}

void View::buildResources(boo::IGraphicsDataFactory::Context& ctx, ViewResources& res)
{
    m_viewVertBlockBuf = res.m_viewRes.m_bufPool.allocateBlock(res.m_factory);
    m_bgVertsBinding.init(ctx, res, 4, m_viewVertBlockBuf);
}

View::View(ViewResources& res)
: m_rootView(*static_cast<RootView*>(this)),
  m_parentView(*static_cast<RootView*>(this)) {}

View::View(ViewResources& res, View& parentView)
: m_rootView(parentView.rootView()),
  m_parentView(parentView) {}

void View::updateSize()
{
    resized(m_rootView.rootRect(), m_subRect);
}

void View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    m_subRect = sub;
    m_viewVertBlock.setViewRect(root, sub);
    m_bgRect[0].m_pos.assign(0.f, sub.size[1], 0.f);
    m_bgRect[1].m_pos.assign(0.f, 0.f, 0.f);
    m_bgRect[2].m_pos.assign(sub.size[0], sub.size[1], 0.f);
    m_bgRect[3].m_pos.assign(sub.size[0], 0.f, 0.f);
    if (m_viewVertBlockBuf)
        m_viewVertBlockBuf.access().finalAssign(m_viewVertBlock);
    m_bgVertsBinding.load<decltype(m_bgRect)>(m_bgRect);
}

void View::resized(const ViewBlock& vb, const boo::SWindowRect& sub)
{
    m_subRect = sub;
    m_bgRect[0].m_pos.assign(0.f, sub.size[1], 0.f);
    m_bgRect[1].m_pos.assign(0.f, 0.f, 0.f);
    m_bgRect[2].m_pos.assign(sub.size[0], sub.size[1], 0.f);
    m_bgRect[3].m_pos.assign(sub.size[0], 0.f, 0.f);
    if (m_viewVertBlockBuf)
        m_viewVertBlockBuf.access().finalAssign(vb);
    m_bgVertsBinding.load<decltype(m_bgRect)>(m_bgRect);
}

void View::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_bgVertsBinding.m_shaderBinding)
    {
        gfxQ->setShaderDataBinding(m_bgVertsBinding);
        gfxQ->draw(0, 4);
    }
}

void View::commitResources(ViewResources& res, const boo::FactoryCommitFunc& commitFunc)
{
    res.m_factory->commitTransaction(commitFunc BooTrace);
}

void View::VertexBufferBindingSolid::init(boo::IGraphicsDataFactory::Context& ctx,
                                          ViewResources& res, size_t count,
                                          const hecl::UniformBufferPool<ViewBlock>::Token& viewBlockBuf)
{
    m_vertsBuf = res.m_viewRes.m_solidPool.allocateBlock(res.m_factory, count);
    auto vBufInfo = m_vertsBuf.getBufferInfo();
    auto uBufInfo = viewBlockBuf.getBufferInfo();

    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uBufInfo.first.get()};
    size_t bufOffs[] = {size_t(uBufInfo.second)};
    size_t bufSizes[] = {sizeof(ViewBlock)};

    m_shaderBinding = ctx.newShaderDataBinding(res.m_viewRes.m_solidShader,
                                               vBufInfo.first.get(), nullptr,
                                               nullptr, 1, bufs, nullptr, bufOffs,
                                               bufSizes, 0, nullptr, nullptr, nullptr, vBufInfo.second);
}

void View::VertexBufferBindingTex::init(boo::IGraphicsDataFactory::Context& ctx,
                                        ViewResources& res, size_t count,
                                        const hecl::UniformBufferPool<ViewBlock>::Token& viewBlockBuf,
                                        const boo::ObjToken<boo::ITexture>& texture)
{
    m_vertsBuf = res.m_viewRes.m_texPool.allocateBlock(res.m_factory, count);
    auto vBufInfo = m_vertsBuf.getBufferInfo();
    auto uBufInfo = viewBlockBuf.getBufferInfo();

    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uBufInfo.first.get()};
    size_t bufOffs[] = {size_t(uBufInfo.second)};
    size_t bufSizes[] = {sizeof(ViewBlock)};
    boo::ObjToken<boo::ITexture> tex[] = {texture};

    m_shaderBinding = ctx.newShaderDataBinding(res.m_viewRes.m_texShader,
                                               vBufInfo.first.get(), nullptr,
                                               nullptr, 1, bufs, nullptr, bufOffs,
                                               bufSizes, 1, tex, nullptr, nullptr, vBufInfo.second);
}


}
