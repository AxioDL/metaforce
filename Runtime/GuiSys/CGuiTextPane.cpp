#include "CGuiTextPane.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CFontImageDef.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CGraphicsPalette.hpp"

namespace urde
{

CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, float xDim, float zDim,
                           const zeus::CVector3f& vec, TResId fontId, const CGuiTextProperties& props,
                           const zeus::CColor& fontCol, const zeus::CColor& outlineCol,
                           s32 extentX, s32 extentY)
: CGuiPane(parms, xDim, zDim, vec), x114_textSupport(fontId, props, fontCol, outlineCol,
                                                     zeus::CColor::skWhite, extentX, extentY,
                                                     &parms.x0_frame->GetGuiSys().GetResStore()) {}

void CGuiTextPane::Update(float dt)
{
    CGuiWidget::Update(dt);
    x114_textSupport.Update(dt);
}

bool CGuiTextPane::GetIsFinishedLoadingWidgetSpecific() const
{
    return x114_textSupport.GetIsTextSupportFinishedLoading();
}

void CGuiTextPane::SetDimensions(const zeus::CVector2f& dim, bool initVBO)
{
    CGuiPane::SetDimensions(dim, initVBO);
    if (initVBO)
        InitializeBuffers();
}

void CGuiTextPane::ScaleDimensions(const zeus::CVector3f& scale)
{
}

void CGuiTextPane::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (!GetIsVisible())
        return;

    zeus::CVector2f dims = GetDimensions();

    if (x114_textSupport.x28_extentX)
        dims.x /= float(x114_textSupport.x28_extentX);
    else
        dims.x = 0.f;

    if (x114_textSupport.x2c_extentY)
        dims.y /= float(x114_textSupport.x2c_extentY);
    else
        dims.y = 0.f;

    zeus::CTransform local =
        zeus::CTransform::Translate(x100_verts.front().m_pos + x108_scaleCenter) *
        zeus::CTransform::Scale(dims.x, 1.f, dims.y);
    CGraphics::SetModelMatrix(x34_worldXF * local);

    zeus::CColor geomCol = xb4_;
    geomCol.a *= parms.x0_alphaMod;
    ((CGuiTextPane*)this)->x114_textSupport.SetGeometryColor(geomCol);

    CGraphics::SetDepthWriteMode(xf6_31_depthTest, ERglEnum::LEqual, xf7_24_depthWrite);

    switch (xc4_drawFlags)
    {
    case EGuiModelDrawFlags::Shadeless:
    case EGuiModelDrawFlags::Opaque:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One,
                                ERglBlendFactor::Zero, ERglLogicOp::Clear);
        x114_textSupport.Render();
        break;
    case EGuiModelDrawFlags::Alpha:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        x114_textSupport.Render();
        break;
    case EGuiModelDrawFlags::Additive:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::One, ERglLogicOp::Clear);
        x114_textSupport.Render();
        break;
    case EGuiModelDrawFlags::AlphaAdditiveOverdraw:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        x114_textSupport.Render();
        ((CGuiTextPane*)this)->x114_textSupport.SetGeometryColor
            (geomCol * zeus::CColor(geomCol.a, geomCol.a, geomCol.a, 1.f));
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One,
                                ERglBlendFactor::One, ERglLogicOp::Clear);
        x114_textSupport.Render();
        break;
    }
}

CGuiTextPane* CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float xDim = in.readFloatBig();
    float zDim = in.readFloatBig();
    zeus::CVector3f vec;
    vec.readBig(in);
    u32 fontId = in.readUint32Big();
    bool wordWrap = in.readBool();
    bool vertical = in.readBool();
    EJustification justification = EJustification(in.readUint32Big());
    EVerticalJustification vJustification = EVerticalJustification(in.readUint32Big());
    CGuiTextProperties props(wordWrap, vertical, false, justification, vJustification);
    zeus::CColor fontCol;
    fontCol.readRGBABig(in);
    zeus::CColor outlineCol;
    outlineCol.readRGBABig(in);
    int extentX = in.readFloatBig();
    int extentY = in.readFloatBig();
    return new CGuiTextPane(parms, xDim, zDim, vec, fontId, props,
                            fontCol, outlineCol, extentX, extentY);
}

}
