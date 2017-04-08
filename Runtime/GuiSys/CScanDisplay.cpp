#include "CScanDisplay.hpp"
#include "zeus/CTransform.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{
void CScanDisplay::CDataDot::Update(float dt)
{
    if (x20_ > 0.f)
    {
        float diff = x20_ - dt;
        if (diff < 0.f)
            x20_ = 0.f;
        else
            x20_ = diff;
        float d = x20_ / x1c_;
        xc_ = (x14_ * (x4_ * d)) + (1.f - x1c_ > 0.f ? d : 0.f);
    }

    if (x24_ > x28_)
    {
        float tmp = -((2.0f * x24_) - dt);
        x24_ = (tmp >= x28_ ? x28_ : tmp);
    }
    else if (x24_ < x28_)
    {
        float tmp = (2.0 * x24_) + x28_;
        x24_ = (tmp >= x28_ ? x28_ : tmp);
    }
}

void CScanDisplay::CDataDot::Draw(const zeus::CColor& col, float f1) const
{
    if (x24_ == 0.f)
       return;

    if (x0_ == EDotState::One)
    {
        zeus::CTransform xf = zeus::CTransform::Translate(xc_.x, 0.f, xc_.y);
#if 0
        g_Renderer->SetModelMatrix(xf);
        CGraphics::StreamBegin(ERglPrimitive::TriangleStrip);
        zeus::Comp8 r, g, b, a;
        col.toRGBA8(r, g, b, a);
        CGraphics::StreamColor({col.r, col.g, col.b, x24_ * float(a)});
        CGraphics::StreamTexCoord(0.f, 1.f);
        CGraphics::StreamVertex(zeus::CVector3f{-f1, 0, f1});
        CGraphics::StreamTexCoord(0.f, 0.f);
        CGraphics::StreamVertex(zeus::CVector3f{-f1, 0, -f1});
        CGraphics::StreamTexCoord(1.f, 1.f);
        CGraphics::StreamVertex(zeus::CVector3f{f1, 0, f1});
        CGraphics::StreamTexCoord(1.f, 0.f);
        CGraphics::StreamVertex(zeus::CVector3f{f1, 0, -f1});
        CGraphics::StreamEnd();
#endif
    }
}

void CScanDisplay::CDataDot::StartTransitionTo(const zeus::CVector2f& vec, float f1)
{
    x20_ = f1;
    x1c_ = f1;
    x4_ = xc_;
    x14_ = vec;
}

void CScanDisplay::CDataDot::SetDestPosition(const zeus::CVector2f& pos)
{
    if (x20_ <= 0.f)
        xc_ = pos;
    else
        x14_ = pos;
}

void CScanDisplay::ProcessInput(const CFinalInput& input)
{

}

void CScanDisplay::StartScan(TUniqueId id, const CScannableObjectInfo& scanInfo, CGuiTextPane* message,
                             CGuiTextPane* scrollMessage, CGuiWidget* textGroup, CGuiModel* xmark,
                             CGuiModel* abutton, CGuiModel* dash, float scanTime)
{

}

void CScanDisplay::StopScan()
{

}

void CScanDisplay::InitializeFrame(float)
{

}

void CScanDisplay::Update(float, float)
{

}

void CScanDisplay::SetBackgroundBucketOccluded(s32, float)
{

}

bool CScanDisplay::PanelCoversBucketBackground(CScannableObjectInfo::EPanelType, s32)
{
    return false;
}

void CScanDisplay::Draw() const
{

}
}
