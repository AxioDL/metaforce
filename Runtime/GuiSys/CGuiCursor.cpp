#include "Runtime/GuiSys/CGuiCursor.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"

namespace urde {
namespace {
constexpr char skGuiCursorPanName[] = "CMDL_GuiCursorPan";
constexpr char skGuiCursorReticleName[] = "CMDL_GuiCursorReticle";
} // Anonymous namespace

CGuiCursorTransfer::CGuiCursorTransfer(const CGuiCursorTransfer& other)
: x0_(other.x0_)
, x4_(other.x4_)
, x8_(other.x8_)
, x14_(other.x14_)
, x18_(other.x18_)
, x1c_(other.x1c_)
, x20_(other.x20_)
, x24_(other.x24_)
, x28_(other.x28_)
, x2c_(other.x2c_)
, x30_(other.x30_)
, x34_(other.x34_)
, x38_(other.x38_)
, x3c_(other.x3c_) {}

CGuiCursorTransfer::CGuiCursorTransfer(float f1, float f2, float f3, float f4) {
  // TODO
}

CGuiCursor::CGuiCursor(const CGuiCursorTransfer& t1, const CGuiCursorTransfer& t2, ECursorState cursorState)
: x0_cursorReticle(g_SimplePool->GetObj(GetCursorName(cursorState)))
, xc_cursorPan(g_SimplePool->GetObj(skGuiCursorPanName))
, x2c_(t1)
, x6c_(t2)
, xe0_cursorColor(GetCursorColor(cursorState)) {
  const zeus::CAABox& box = x0_cursorReticle.GetObj()->GetInstance().GetAABB();
  float x = 30.f / (box.max.x() - box.min.x());
  x20_ = zeus::CVector3f(x, 1.f, -1.f * x);
}

void FUN_800d7eec() {
  const boo::SWindowRect& rect = CGraphics::GetViewport();
  CGraphics::SetOrtho(rect.location[0], rect.location[0] + rect.size[0],
      rect.location[1] + rect.size[1], rect.location[1], -1.f, 1.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform::Translate(-0.5f * rect.size[0], 0.f, -0.5f * rect.size[1]));
}

zeus::CVector3f CGuiCursor::ProjectCursorPosition() {
  const SClipScreenRect& rect = CGraphics::g_CroppedViewport;
  float w = 0.5f * rect.xc_width, h = 0.5f * rect.x10_height;
  if (-h <= 0.f) {
    h = h * x18_.y();
  }
  if (-w <= 0.f) {
    w = w * x18_.x();
  }
  if (-h - h <= 0.f) {
    h = -h;
  }
  if (-w - w <= 0.f) {
    w = -w;
  }
  return zeus::CVector3f(std::floor(0.5f + w), 0.f, std::floor(0.5f + h));
}

void CGuiCursor::Draw() {
  float alpha = xb4_ * xc8_;
  if (xe8_ && 0.f < alpha) { // && DAT_805c2580 == '\0' ?
    FUN_800d7eec();
    const zeus::CVector3f pos = ProjectCursorPosition();
    const zeus::CTransform scale = zeus::CTransform::Scale(x20_);
    const zeus::CTransform translate = zeus::CTransform::Translate(pos);
    zeus::CTransform t = translate * scale;
    if (xb0_ < 0.5f) {
      const zeus::CTransform rotX = zeus::CTransform::RotateX(zeus::degToRad(xd8_));
      const zeus::CTransform rotY = zeus::CTransform::RotateY(zeus::degToRad(xdc_));
      const zeus::CTransform rot = rotY * rotX;
      t = t * rot;
    }
    CGraphics::SetModelMatrix(t);
    CGraphics::SetCullMode(ERglCullMode::None);

    zeus::CColor x1a4 = xe0_cursorColor;
    x1a4.a() = alpha;

    if (0.5f <= xb0_) {
      zeus::CColor color = xe0_cursorColor;
      color.a() = 2.f * (xb0_ - 0.5f);
      if (xe4_ == ECursorState::x1) {
        CModelFlags flags(5, 0, 2, color);
        xc_cursorPan->Draw(flags);
      } else {
        CModelFlags flags(7, 0, 2, color);
        xc_cursorPan->Draw(flags);
      }
    } else {
      zeus::CColor color = xe0_cursorColor;
      color.a() = 1.f - (2.f * xb0_);
      if (xe4_ == ECursorState::x1) {
        CModelFlags flags(5, 0, 2, color);
        x0_cursorReticle->Draw(flags);
      } else {
        CModelFlags flags(7, 0, 2, color);
        x0_cursorReticle->Draw(flags);
      }
    }
  }
}

void CGuiCursor::Update(double dt) {

}

const char* CGuiCursor::GetCursorName(ECursorState state) {
  return state < ECursorState::x2 ? skGuiCursorReticleName : nullptr;
}

const zeus::CColor CGuiCursor::GetCursorColor(ECursorState state) {
  return zeus::Comp32(state == ECursorState::x1 ? 0x83c1ff7e : 0x6e72fbb5);
}
} // namespace urde
