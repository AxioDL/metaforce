#include "specter/ModalWindow.hpp"

#include <algorithm>

#include "specter/MultiLineTextView.hpp"
#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"

#include <boo/System.hpp>

namespace specter {

#define WIRE_START 0
#define WIRE_FRAMES 40
#define SOLID_START 30
#define SOLID_FRAMES 20
#define CONTENT_START 40
#define CONTENT_FRAMES 10

#define LINE_WIDTH 2
#define CONTENT_MARGIN 10
#define WINDOW_MIN_DIM 16

void ModalWindow::setLineVerts(int width, int height, float pf, float t) {
  const std::pair<int, int> margin = m_cornersOutline[0]->queryGlyphDimensions(0);
  const float t1 = std::clamp(t * 2.f, 0.f, 1.f);
  const float t2 = std::clamp(t * 2.f - 1.f, 0.f, 1.f);

  float lineLeft = 0;
  float lineRight = pf * LINE_WIDTH;
  float lineTop = height - margin.second;
  float lineBottom = margin.second;
  m_verts.lineVerts[0].m_pos.assign(lineLeft, lineTop, 0);
  m_verts.lineVerts[1].m_pos = zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineLeft, lineBottom, 0}, t1);
  m_verts.lineVerts[2].m_pos.assign(lineRight, lineTop, 0);
  m_verts.lineVerts[3].m_pos = zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineRight, lineBottom, 0}, t1);
  m_verts.lineVerts[4].m_pos = m_verts.lineVerts[3].m_pos;

  lineLeft = margin.first;
  lineRight = width - margin.first;
  lineTop = height;
  lineBottom = height - pf * LINE_WIDTH;
  m_verts.lineVerts[5].m_pos.assign(lineLeft, lineTop, 0);
  m_verts.lineVerts[6].m_pos = m_verts.lineVerts[5].m_pos;
  m_verts.lineVerts[7].m_pos.assign(lineLeft, lineBottom, 0);
  m_verts.lineVerts[8].m_pos = zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineRight, lineTop, 0}, t1);
  m_verts.lineVerts[9].m_pos = zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineRight, lineBottom, 0}, t1);
  m_verts.lineVerts[10].m_pos = m_verts.lineVerts[9].m_pos;

  lineLeft = width - pf * LINE_WIDTH;
  lineRight = width;
  lineTop = height - margin.second;
  lineBottom = margin.second;
  m_verts.lineVerts[11].m_pos.assign(lineLeft, lineTop, 0);
  m_verts.lineVerts[12].m_pos = m_verts.lineVerts[11].m_pos;
  m_verts.lineVerts[13].m_pos = zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineLeft, lineBottom, 0}, t2);
  m_verts.lineVerts[14].m_pos.assign(lineRight, lineTop, 0);
  m_verts.lineVerts[15].m_pos = zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineRight, lineBottom, 0}, t2);
  m_verts.lineVerts[16].m_pos = m_verts.lineVerts[15].m_pos;

  lineLeft = margin.first;
  lineRight = width - margin.first;
  lineTop = pf * LINE_WIDTH;
  lineBottom = 0;
  m_verts.lineVerts[17].m_pos.assign(lineLeft, lineTop, 0);
  m_verts.lineVerts[18].m_pos = m_verts.lineVerts[17].m_pos;
  m_verts.lineVerts[19].m_pos.assign(lineLeft, lineBottom, 0);
  m_verts.lineVerts[20].m_pos = zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineRight, lineTop, 0}, t2);
  m_verts.lineVerts[21].m_pos = zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineRight, lineBottom, 0}, t2);
}

void ModalWindow::setLineVertsOut(int width, int height, float pf, float t) {
  const std::pair<int, int> margin = m_cornersOutline[0]->queryGlyphDimensions(0);
  const float t1 = std::clamp(t * 2.f - 1.f, 0.f, 1.f);
  const float t2 = std::clamp(t * 2.f, 0.f, 1.f);

  float lineLeft = 0;
  float lineRight = pf * LINE_WIDTH;
  float lineTop = height - margin.second;
  float lineBottom = margin.second;
  m_verts.lineVerts[0].m_pos = zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineLeft, lineTop, 0}, t1);
  m_verts.lineVerts[1].m_pos.assign(lineLeft, lineBottom, 0);
  m_verts.lineVerts[2].m_pos = zeus::CVector3f::lerp({lineRight, lineBottom, 0}, {lineRight, lineTop, 0}, t1);
  m_verts.lineVerts[3].m_pos.assign(lineRight, lineBottom, 0);
  m_verts.lineVerts[4].m_pos = m_verts.lineVerts[3].m_pos;

  lineLeft = margin.first;
  lineRight = width - margin.first;
  lineTop = height;
  lineBottom = height - pf * LINE_WIDTH;
  m_verts.lineVerts[5].m_pos = zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineLeft, lineTop, 0}, t1);
  m_verts.lineVerts[6].m_pos = m_verts.lineVerts[5].m_pos;
  m_verts.lineVerts[7].m_pos = zeus::CVector3f::lerp({lineRight, lineBottom, 0}, {lineLeft, lineBottom, 0}, t1);
  m_verts.lineVerts[8].m_pos.assign(lineRight, lineTop, 0);
  m_verts.lineVerts[9].m_pos.assign(lineRight, lineBottom, 0);
  m_verts.lineVerts[10].m_pos = m_verts.lineVerts[9].m_pos;

  lineLeft = width - pf * LINE_WIDTH;
  lineRight = width;
  lineTop = height - margin.second;
  lineBottom = margin.second;
  m_verts.lineVerts[11].m_pos = zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineLeft, lineTop, 0}, t2);
  m_verts.lineVerts[12].m_pos = m_verts.lineVerts[11].m_pos;
  m_verts.lineVerts[13].m_pos.assign(lineLeft, lineBottom, 0);
  m_verts.lineVerts[14].m_pos = zeus::CVector3f::lerp({lineRight, lineBottom, 0}, {lineRight, lineTop, 0}, t2);
  m_verts.lineVerts[15].m_pos.assign(lineRight, lineBottom, 0);
  m_verts.lineVerts[16].m_pos = m_verts.lineVerts[15].m_pos;

  lineLeft = margin.first;
  lineRight = width - margin.first;
  lineTop = pf * LINE_WIDTH;
  lineBottom = 0;
  m_verts.lineVerts[17].m_pos = zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineLeft, lineTop, 0}, t2);
  m_verts.lineVerts[18].m_pos = m_verts.lineVerts[17].m_pos;
  m_verts.lineVerts[19].m_pos = zeus::CVector3f::lerp({lineRight, lineBottom, 0}, {lineLeft, lineBottom, 0}, t2);
  m_verts.lineVerts[20].m_pos.assign(lineRight, lineTop, 0);
  m_verts.lineVerts[21].m_pos.assign(lineRight, lineBottom, 0);
}

void ModalWindow::setLineColors(float t) {
  const float t1 = std::clamp(t * 2.f, 0.f, 1.f);
  const float t2 = std::clamp(t * 2.f - 1.f, 0.f, 1.f);
  const float t3 = std::clamp(t * 2.f - 2.f, 0.f, 1.f);

  const zeus::CColor c1 = zeus::CColor::lerp(m_line1, m_line2, t1);
  const zeus::CColor c2 = zeus::CColor::lerp(m_line1, m_line2, t2);
  const zeus::CColor c3 = zeus::CColor::lerp(m_line1, m_line2, t3);

  m_cornersOutline[0]->colorGlyphs(c1);
  if (t < 0.5) {
    m_cornersOutline[1]->colorGlyphs(zeus::skClear);
    m_cornersOutline[2]->colorGlyphs(zeus::skClear);
    m_cornersOutline[3]->colorGlyphs(zeus::skClear);
  } else if (t < 1.0) {
    m_cornersOutline[1]->colorGlyphs(c2);
    m_cornersOutline[2]->colorGlyphs(zeus::skClear);
    m_cornersOutline[3]->colorGlyphs(c2);
  } else {
    m_cornersOutline[1]->colorGlyphs(c2);
    m_cornersOutline[2]->colorGlyphs(c3);
    m_cornersOutline[3]->colorGlyphs(c2);
  }

  m_verts.lineVerts[0].m_color = c1;
  m_verts.lineVerts[1].m_color = c2;
  m_verts.lineVerts[2].m_color = m_verts.lineVerts[0].m_color;
  m_verts.lineVerts[3].m_color = m_verts.lineVerts[1].m_color;
  m_verts.lineVerts[4].m_color = m_verts.lineVerts[3].m_color;

  m_verts.lineVerts[5].m_color = c1;
  m_verts.lineVerts[6].m_color = m_verts.lineVerts[5].m_color;
  m_verts.lineVerts[7].m_color = m_verts.lineVerts[6].m_color;
  m_verts.lineVerts[8].m_color = c2;
  m_verts.lineVerts[9].m_color = m_verts.lineVerts[8].m_color;
  m_verts.lineVerts[10].m_color = m_verts.lineVerts[9].m_color;

  m_verts.lineVerts[11].m_color = c2;
  m_verts.lineVerts[12].m_color = m_verts.lineVerts[11].m_color;
  m_verts.lineVerts[13].m_color = c3;
  m_verts.lineVerts[14].m_color = m_verts.lineVerts[12].m_color;
  m_verts.lineVerts[15].m_color = m_verts.lineVerts[13].m_color;
  m_verts.lineVerts[16].m_color = m_verts.lineVerts[15].m_color;

  m_verts.lineVerts[17].m_color = c2;
  m_verts.lineVerts[18].m_color = m_verts.lineVerts[17].m_color;
  m_verts.lineVerts[19].m_color = m_verts.lineVerts[18].m_color;
  m_verts.lineVerts[20].m_color = c3;
  m_verts.lineVerts[21].m_color = m_verts.lineVerts[20].m_color;
}

void ModalWindow::setLineColorsOut(float t) {
  const float t1 = std::clamp(t * 2.f, 0.f, 1.f);
  const float t2 = std::clamp(t * 2.f - 1.f, 0.f, 1.f);
  const float t3 = std::clamp(t * 2.f - 2.f, 0.f, 1.f);

  const zeus::CColor c1 = zeus::CColor::lerp(m_line2Clear, m_line2, t1);
  const zeus::CColor c2 = zeus::CColor::lerp(m_line2Clear, m_line2, t2);
  const zeus::CColor c3 = zeus::CColor::lerp(m_line2Clear, m_line2, t3);

  m_cornersOutline[2]->colorGlyphs(c1);
  if (t < 0.5) {
    m_cornersOutline[1]->colorGlyphs(zeus::skClear);
    m_cornersOutline[0]->colorGlyphs(zeus::skClear);
    m_cornersOutline[3]->colorGlyphs(zeus::skClear);
  } else if (t < 1.0) {
    m_cornersOutline[1]->colorGlyphs(c2);
    m_cornersOutline[0]->colorGlyphs(zeus::skClear);
    m_cornersOutline[3]->colorGlyphs(c2);
  } else {
    m_cornersOutline[1]->colorGlyphs(c2);
    m_cornersOutline[0]->colorGlyphs(c3);
    m_cornersOutline[3]->colorGlyphs(c2);
  }

  m_verts.lineVerts[0].m_color = c3;
  m_verts.lineVerts[1].m_color = c2;
  m_verts.lineVerts[2].m_color = m_verts.lineVerts[0].m_color;
  m_verts.lineVerts[3].m_color = m_verts.lineVerts[1].m_color;
  m_verts.lineVerts[4].m_color = m_verts.lineVerts[3].m_color;

  m_verts.lineVerts[5].m_color = c3;
  m_verts.lineVerts[6].m_color = m_verts.lineVerts[5].m_color;
  m_verts.lineVerts[7].m_color = m_verts.lineVerts[6].m_color;
  m_verts.lineVerts[8].m_color = c2;
  m_verts.lineVerts[9].m_color = m_verts.lineVerts[8].m_color;
  m_verts.lineVerts[10].m_color = m_verts.lineVerts[9].m_color;

  m_verts.lineVerts[11].m_color = c2;
  m_verts.lineVerts[12].m_color = m_verts.lineVerts[11].m_color;
  m_verts.lineVerts[13].m_color = c1;
  m_verts.lineVerts[14].m_color = m_verts.lineVerts[12].m_color;
  m_verts.lineVerts[15].m_color = m_verts.lineVerts[13].m_color;
  m_verts.lineVerts[16].m_color = m_verts.lineVerts[15].m_color;

  m_verts.lineVerts[17].m_color = c2;
  m_verts.lineVerts[18].m_color = m_verts.lineVerts[17].m_color;
  m_verts.lineVerts[19].m_color = m_verts.lineVerts[18].m_color;
  m_verts.lineVerts[20].m_color = c1;
  m_verts.lineVerts[21].m_color = m_verts.lineVerts[20].m_color;
}

void ModalWindow::setFillVerts(int width, int height, float pf) {
  std::pair<int, int> margin = m_cornersFilled[0]->queryGlyphDimensions(0);

  float fillLeft = pf * LINE_WIDTH;
  float fillRight = width - pf * LINE_WIDTH;
  float fillTop = height - margin.second;
  float fillBottom = margin.second;
  m_verts.fillVerts[0].m_pos.assign(fillLeft, fillTop, 0);
  m_verts.fillVerts[1].m_pos.assign(fillLeft, fillBottom, 0);
  m_verts.fillVerts[2].m_pos.assign(fillRight, fillTop, 0);
  m_verts.fillVerts[3].m_pos.assign(fillRight, fillBottom, 0);
  m_verts.fillVerts[4].m_pos = m_verts.fillVerts[3].m_pos;

  fillLeft = margin.first;
  fillRight = width - margin.first;
  fillTop = height - pf * LINE_WIDTH;
  fillBottom = height - margin.second;
  m_verts.fillVerts[5].m_pos.assign(fillLeft, fillTop, 0);
  m_verts.fillVerts[6].m_pos = m_verts.fillVerts[5].m_pos;
  m_verts.fillVerts[7].m_pos.assign(fillLeft, fillBottom, 0);
  m_verts.fillVerts[8].m_pos.assign(fillRight, fillTop, 0);
  m_verts.fillVerts[9].m_pos.assign(fillRight, fillBottom, 0);
  m_verts.fillVerts[10].m_pos = m_verts.fillVerts[9].m_pos;

  fillLeft = margin.first;
  fillRight = width - margin.first;
  fillTop = margin.second;
  fillBottom = pf * LINE_WIDTH;
  m_verts.fillVerts[11].m_pos.assign(fillLeft, fillTop, 0);
  m_verts.fillVerts[12].m_pos = m_verts.fillVerts[11].m_pos;
  m_verts.fillVerts[13].m_pos.assign(fillLeft, fillBottom, 0);
  m_verts.fillVerts[14].m_pos.assign(fillRight, fillTop, 0);
  m_verts.fillVerts[15].m_pos.assign(fillRight, fillBottom, 0);
}

void ModalWindow::setFillColors(float t) {
  t = std::clamp(t, 0.f, 1.f);
  zeus::CColor color = zeus::CColor::lerp(m_windowBgClear, m_windowBg, t);

  for (int i = 0; i < 16; ++i)
    m_verts.fillVerts[i].m_color = color;
  for (int i = 0; i < 4; ++i)
    m_cornersFilled[i]->colorGlyphs(color);
}

ModalWindow::ModalWindow(ViewResources& res, View& parentView, const RectangleConstraint& constraint)
: ModalWindow(res, parentView, constraint, res.themeData().splashBackground()) {}

ModalWindow::ModalWindow(ViewResources& res, View& parentView, const RectangleConstraint& constraint,
                         const zeus::CColor& bgColor)
: View(res, parentView)
, m_constraint(constraint)
, m_windowBg(bgColor)
, m_windowBgClear(m_windowBg)
, m_line1(res.themeData().splash1())
, m_line2(res.themeData().splash2())
, m_line2Clear(m_line2) {
  m_windowBgClear[3] = 0.0;
  m_line2Clear[3] = 0.0;

  res.m_factory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
    buildResources(ctx, res);
    m_viewBlockBuf = res.m_viewRes.m_bufPool.allocateBlock(res.m_factory);
    m_vertsBinding.init(ctx, res, 38, m_viewBlockBuf);
    return true;
  } BooTrace);

  for (int i = 0; i < 4; ++i) {
    m_cornersOutline[i].reset(
        new specter::TextView(res, *this, res.m_curveFont, specter::TextView::Alignment::Left, 1));
    m_cornersFilled[i].reset(new specter::TextView(res, *this, res.m_curveFont, specter::TextView::Alignment::Left, 1));
  }
  m_cornersOutline[0]->typesetGlyphs(L"\xF4F0");
  m_cornersFilled[0]->typesetGlyphs(L"\xF4F1", res.themeData().splashBackground());
  m_cornersOutline[1]->typesetGlyphs(L"\xF4F2");
  m_cornersFilled[1]->typesetGlyphs(L"\xF4F3", res.themeData().splashBackground());
  m_cornersOutline[2]->typesetGlyphs(L"\xF4F4");
  m_cornersFilled[2]->typesetGlyphs(L"\xF4F5", res.themeData().splashBackground());
  m_cornersOutline[3]->typesetGlyphs(L"\xF4F6");
  m_cornersFilled[3]->typesetGlyphs(L"\xF4F7", res.themeData().splashBackground());

  setLineColors(0.0);
  setFillColors(0.0);

  _loadVerts();
}

ModalWindow::~ModalWindow() = default;

static float CubicEase(float t) {
  t *= 2.f;
  if (t < 1)
    return 1.f / 2.f * t * t * t;
  t -= 2.f;
  return 1.f / 2.f * (t * t * t + 2.f);
}

void ModalWindow::think() {
  specter::ViewResources& res = rootView().viewRes();
  float pf = res.pixelFactor();

  switch (m_phase) {
  case Phase::BuildIn: {
    bool loadVerts = false;
    int doneCount = 0;
    if (m_frame > WIRE_START) {
      float wt = (m_frame - WIRE_START) / float(WIRE_FRAMES);
      wt = std::clamp(wt, 0.f, 2.f);
      m_lineTime = CubicEase(wt);
      setLineVerts(m_width, m_height, pf, m_lineTime);
      setLineColors(wt);
      if (wt == 2.f)
        ++doneCount;
      loadVerts = true;
    }
    if (m_frame > SOLID_START) {
      float ft = (m_frame - SOLID_START) / float(SOLID_FRAMES);
      ft = std::clamp(ft, 0.f, 2.f);
      setFillColors(ft);
      if (ft == 2.f)
        ++doneCount;
      loadVerts = true;
    }
    if (res.fontCacheReady() && m_frame > CONTENT_START) {
      if (!m_contentStartFrame)
        m_contentStartFrame = m_frame;
      float tt = (m_frame - m_contentStartFrame) / float(CONTENT_FRAMES);
      tt = std::clamp(tt, 0.f, 1.f);
      updateContentOpacity(tt);
      if (tt == 1.f)
        ++doneCount;
    }
    if (doneCount == 3)
      m_phase = Phase::Showing;
    if (loadVerts)
      _loadVerts();
    ++m_frame;
    break;
  }
  case Phase::ResWait: {
    if (res.fontCacheReady()) {
      updateContentOpacity(1.0);
      m_phase = Phase::Showing;
    }
    break;
  }
  case Phase::BuildOut: {
    {
      float wt = (WIRE_FRAMES - m_frame) / float(WIRE_FRAMES);
      wt = std::clamp(wt, 0.f, 1.f);
      m_lineTime = CubicEase(wt);
      setLineVertsOut(m_width, m_height, pf, m_lineTime);
      setLineColorsOut(wt);
      if (wt == 0.f)
        m_phase = Phase::Done;
    }
    {
      float ft = (SOLID_FRAMES - m_frame) / float(SOLID_FRAMES);
      ft = std::clamp(ft, 0.f, 1.f);
      setFillColors(ft);
    }
    if (res.fontCacheReady()) {
      float tt = (CONTENT_FRAMES - m_frame) / float(CONTENT_FRAMES);
      tt = std::clamp(tt, 0.f, 1.f);
      updateContentOpacity(tt);
    }
    _loadVerts();
    ++m_frame;
    break;
  }
  default:
    break;
  }
}

bool ModalWindow::skipBuildInAnimation() {
  if (m_phase != Phase::BuildIn)
    return false;

  specter::ViewResources& res = rootView().viewRes();
  float pf = res.pixelFactor();

  m_lineTime = 1.0;
  setLineVerts(m_width, m_height, pf, 1.0);
  setLineColors(2.0);
  setFillColors(2.0);
  _loadVerts();
  m_phase = Phase::ResWait;
  return true;
}

void ModalWindow::close(bool skipAnimation) {
  m_phase = skipAnimation ? Phase::Done : Phase::BuildOut;
  m_frame = 0;
}

void ModalWindow::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  float pf = rootView().viewRes().pixelFactor();

  boo::SWindowRect centerRect = sub;
  std::pair<int, int> constrained =
      m_constraint.solve(root.size[0] - CONTENT_MARGIN * pf * 2, root.size[1] - CONTENT_MARGIN * pf * 2);
  m_width = std::max(constrained.first, int(WINDOW_MIN_DIM * pf));
  m_height = std::max(constrained.second, int(WINDOW_MIN_DIM * pf));
  centerRect.size[0] = m_width;
  centerRect.size[1] = m_height;
  centerRect.location[0] = root.size[0] / 2 - m_width / 2.0;
  centerRect.location[1] = root.size[1] / 2 - m_height / 2.0;
  View::resized(root, centerRect);
  m_viewBlock.setViewRect(root, centerRect);
  m_viewBlockBuf.access().finalAssign(m_viewBlock);

  setLineVerts(m_width, m_height, pf, m_lineTime);
  setFillVerts(m_width, m_height, pf);
  _loadVerts();

  boo::SWindowRect cornerRect = centerRect;
  cornerRect.size[0] = cornerRect.size[1] = 8 * pf;
  cornerRect.location[1] = centerRect.location[1] + m_height - 8 * pf;
  m_cornersOutline[0]->resized(root, cornerRect);
  m_cornersFilled[0]->resized(root, cornerRect);
  cornerRect.location[0] = centerRect.location[0] + m_width - 8 * pf;
  m_cornersOutline[1]->resized(root, cornerRect);
  m_cornersFilled[1]->resized(root, cornerRect);
  cornerRect.location[1] = centerRect.location[1];
  m_cornersOutline[2]->resized(root, cornerRect);
  m_cornersFilled[2]->resized(root, cornerRect);
  cornerRect.location[0] = centerRect.location[0];
  m_cornersOutline[3]->resized(root, cornerRect);
  m_cornersFilled[3]->resized(root, cornerRect);
}

void ModalWindow::draw(boo::IGraphicsCommandQueue* gfxQ) {
  if (m_phase == Phase::Done)
    return;

  gfxQ->setShaderDataBinding(m_vertsBinding);
  gfxQ->draw(0, 22);
  gfxQ->draw(22, 16);

  m_cornersFilled[0]->draw(gfxQ);
  m_cornersFilled[1]->draw(gfxQ);
  m_cornersFilled[2]->draw(gfxQ);
  m_cornersFilled[3]->draw(gfxQ);

  m_cornersOutline[0]->draw(gfxQ);
  m_cornersOutline[1]->draw(gfxQ);
  m_cornersOutline[2]->draw(gfxQ);
  m_cornersOutline[3]->draw(gfxQ);
}

} // namespace specter
