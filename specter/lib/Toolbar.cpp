#include "logvisor/logvisor.hpp"
#include "specter/Toolbar.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"

#define TOOLBAR_PADDING 10

namespace specter {
static logvisor::Module Log("specter::Space");

static const zeus::RGBA32 Tex[] = {{{255, 255, 255, 64}}, {{255, 255, 255, 64}}, {{0, 0, 0, 64}}, {{0, 0, 0, 64}}};

void Toolbar::Resources::init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme) {
  m_shadingTex = ctx.newStaticTexture(4, 1, 1, boo::TextureFormat::RGBA8, boo::TextureClampMode::Repeat, Tex, 16);
}

Toolbar::Toolbar(ViewResources& res, View& parentView, Position tbPos, unsigned units)
: View(res, parentView)
, m_units(units)
, m_nomGauge(res.pixelFactor() * SPECTER_TOOLBAR_GAUGE * units)
, m_padding(res.pixelFactor() * TOOLBAR_PADDING) {
  m_children.reserve(units);
  for (unsigned u = 0; u < units; ++u)
    m_children.emplace_back();
  commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool {
    buildResources(ctx, res);
    m_tbBlockBuf = res.m_viewRes.m_bufPool.allocateBlock(res.m_factory);
    m_vertsBinding.init(ctx, res, 10, m_tbBlockBuf, res.m_toolbarRes.m_shadingTex.get());
    return true;
  });
  setBackground(res.themeData().toolbarBackground());
}

void Toolbar::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.mouseDown(coord, button, mod);
}

void Toolbar::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.mouseUp(coord, button, mod);
}

void Toolbar::mouseMove(const boo::SWindowCoord& coord) {
  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.mouseMove(coord);
}

void Toolbar::mouseEnter(const boo::SWindowCoord& coord) {
  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.mouseEnter(coord);
}

void Toolbar::mouseLeave(const boo::SWindowCoord& coord) {
  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.mouseLeave(coord);
}

void Toolbar::setHorizontalVerts(int width) {
  m_tbVerts[0].m_pos.assign(0, 1 + m_nomGauge, 0);
  m_tbVerts[0].m_uv.assign(0, 0);
  m_tbVerts[1].m_pos.assign(0, -1 + m_nomGauge, 0);
  m_tbVerts[1].m_uv.assign(0.5, 0);
  m_tbVerts[2].m_pos.assign(width, 1 + m_nomGauge, 0);
  m_tbVerts[2].m_uv.assign(0, 0);
  m_tbVerts[3].m_pos.assign(width, -1 + m_nomGauge, 0);
  m_tbVerts[3].m_uv.assign(0.5, 0);
  m_tbVerts[4].m_pos.assign(width, -1 + m_nomGauge, 0);
  m_tbVerts[4].m_uv.assign(0.5, 0);

  m_tbVerts[5].m_pos.assign(0, 1, 0);
  m_tbVerts[5].m_uv.assign(0.5, 0);
  m_tbVerts[6].m_pos.assign(0, 1, 0);
  m_tbVerts[6].m_uv.assign(0.5, 0);
  m_tbVerts[7].m_pos.assign(0, -1, 0);
  m_tbVerts[7].m_uv.assign(1, 0);
  m_tbVerts[8].m_pos.assign(width, 1, 0);
  m_tbVerts[8].m_uv.assign(0.5, 0);
  m_tbVerts[9].m_pos.assign(width, -1, 0);
  m_tbVerts[9].m_uv.assign(1, 0);
}

void Toolbar::setVerticalVerts(int height) {
  m_tbVerts[0].m_pos.assign(-1, height, 0);
  m_tbVerts[0].m_uv.assign(0, 0);
  m_tbVerts[1].m_pos.assign(-1, 0, 0);
  m_tbVerts[1].m_uv.assign(0, 0);
  m_tbVerts[2].m_pos.assign(1, height, 0);
  m_tbVerts[2].m_uv.assign(0.5, 0);
  m_tbVerts[3].m_pos.assign(1, 0, 0);
  m_tbVerts[3].m_uv.assign(0.5, 0);
  m_tbVerts[4].m_pos.assign(1, 0, 0);
  m_tbVerts[4].m_uv.assign(0.5, 0);

  m_tbVerts[5].m_pos.assign(-1 + m_nomGauge, height, 0);
  m_tbVerts[5].m_uv.assign(0.5, 0);
  m_tbVerts[6].m_pos.assign(-1 + m_nomGauge, height, 0);
  m_tbVerts[6].m_uv.assign(0.5, 0);
  m_tbVerts[7].m_pos.assign(-1 + m_nomGauge, 0, 0);
  m_tbVerts[7].m_uv.assign(0.5, 0);
  m_tbVerts[8].m_pos.assign(1 + m_nomGauge, height, 0);
  m_tbVerts[8].m_uv.assign(1, 0);
  m_tbVerts[9].m_pos.assign(1 + m_nomGauge, 0, 0);
  m_tbVerts[9].m_uv.assign(1, 0);
}

void Toolbar::push_back(View* v, unsigned unit) {
  if (unit >= m_units)
    Log.report(logvisor::Fatal, "unit %u out of range %u", unit, m_units);
  std::vector<ViewChild<View*>>& u = m_children[unit];
  u.emplace_back();
  u.back().m_view = v;
}

void Toolbar::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  View::resized(root, sub);
  setHorizontalVerts(sub.size[0]);
  m_vertsBinding.load<decltype(m_tbVerts)>(m_tbVerts);
  m_tbBlock.setViewRect(root, sub);
  m_tbBlockBuf.access().finalAssign(m_tbBlock);

  float gaugeUnit = rootView().viewRes().pixelFactor() * SPECTER_TOOLBAR_GAUGE;
  float yOff = 0.0;
  for (std::vector<ViewChild<View*>>& u : m_children) {
    boo::SWindowRect childRect = sub;
    boo::SWindowRect containRect = sub;
    containRect.location[0] += m_padding;
    containRect.size[0] -= m_padding * 2;
    containRect.size[1] = gaugeUnit;
    for (ViewChild<View*>& c : u) {
      c.m_view->containerResized(root, containRect);
      childRect.size[0] = c.m_view->nominalWidth();
      childRect.size[1] = c.m_view->nominalHeight();
      childRect.location[0] += m_padding;
      childRect.location[1] = sub.location[1] + (gaugeUnit - childRect.size[1]) / 2 - 1 + yOff;
      c.m_view->resized(root, childRect);
      childRect.location[0] += childRect.size[0];

      containRect.location[0] += m_padding + childRect.size[0];
      containRect.size[0] -= m_padding + childRect.size[0];
      containRect.size[1] = gaugeUnit;
    }
    yOff += gaugeUnit;
  }
}

void Toolbar::draw(boo::IGraphicsCommandQueue* gfxQ) {
  View::draw(gfxQ);
  gfxQ->setShaderDataBinding(m_vertsBinding);
  gfxQ->draw(0, 10);

  for (std::vector<ViewChild<View*>>& u : m_children)
    for (ViewChild<View*>& c : u)
      c.m_view->draw(gfxQ);
}

} // namespace specter
