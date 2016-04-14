#include "CGuiStaticImage.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

CGuiStaticImage::CGuiStaticImage
    (const CGuiWidgetParms& parms, float xDim, float zDim,
     const zeus::CVector3f& scaleCenter,
     EGuiTextureClampModeHorz clampH, EGuiTextureClampModeVert clampV,
     CGuiStaticImage::EMaterialType matType, ResId txtrId1, ResId txtrId2,
     const std::vector<float>& frame, bool useTexture)
: CGuiPane(parms, xDim, zDim, scaleCenter),
  x114_materialType(matType),
  x120_textureID1(txtrId1),
  x124_textureID2(txtrId2),
  x128_clampH(clampH),
  x12c_clampV(clampV),
  x130_clampedUVs(frame),
  x140_UVs(frame)
{
    CGuiSys& guiSys = parms.x0_frame->GetGuiSys();
    if (useTexture && guiSys.GetUsageMode() != CGuiSys::EUsageMode::Two)
    {
        if ((txtrId1 & 0xffff) != 0xffff)
            x118_texture1 = guiSys.GetResStore().GetObj({SBIG('TXTR'), txtrId1});
        if ((txtrId2 & 0xffff) != 0xffff)
            x11c_texture2 = guiSys.GetResStore().GetObj({SBIG('TXTR'), txtrId2});
    }
    SetDimensions({xDim, zDim}, true);
}

void CGuiStaticImage::ScaleDimensions(const zeus::CVector3f& scale)
{
    CGuiPane::ScaleDimensions(scale);
    zeus::CVector2f dim = GetDimensions();

    float resH1 = x140_UVs[4];
    float resH0 = x140_UVs[0];
    float resV1 = x140_UVs[3];
    float resV0 = x140_UVs[1];

    float f3 = resH1 - resH0;
    float f1 = std::fabs(x100_verts[2].m_pos.x - x100_verts[0].m_pos.x) - dim.x;

    switch (x128_clampH)
    {
    case EGuiTextureClampModeHorz::Right:
        resH1 += f3 * f1 / dim.x;
        break;
    case EGuiTextureClampModeHorz::Left:
        resH0 -= f3 * f1 / dim.x;
        break;
    case EGuiTextureClampModeHorz::Center:
        resH1 += f3 * f1 * 0.5f / dim.x;
        resH0 -= f3 * f1 * 0.5f / dim.x;
        break;
    default: break;
    }

    f3 = resV1 - resV0;
    f1 = std::fabs(x100_verts[0].m_pos.z - x100_verts[1].m_pos.z) - dim.y;

    switch (x12c_clampV)
    {
    case EGuiTextureClampModeVert::Top:
        resV0 -= f3 * f1 / dim.y;
        break;
    case EGuiTextureClampModeVert::Bottom:
        resV1 += f3 * f1 / dim.y;
        break;
    case EGuiTextureClampModeVert::Center:
        resV1 += f3 * f1 * 0.5f / dim.y;
        resV0 -= f3 * f1 * 0.5f / dim.y;
        break;
    default: break;
    }

    x130_clampedUVs[0] = resH0;
    x130_clampedUVs[1] = resV0;
    x130_clampedUVs[2] = resH0;
    x130_clampedUVs[3] = resV1;
    x130_clampedUVs[4] = resH1;
    x130_clampedUVs[5] = resV0;
    x130_clampedUVs[6] = resH1;
    x130_clampedUVs[7] = resV1;
}

void CGuiStaticImage::Draw(const CGuiWidgetDrawParms& parms) const
{
    CGraphics::SetModelMatrix(x34_worldXF * zeus::CTransform::Translate(x108_scaleCenter));
    if (GetIsVisible())
    {
        switch (x114_materialType)
        {
        case EMaterialType::OneTexture:
            x118_texture1->Load(0, CTexture::EClampMode::One);
            break;
        case EMaterialType::TwoTextures:
            x118_texture1->Load(0, CTexture::EClampMode::One);
            x11c_texture2->Load(1, CTexture::EClampMode::One);
            break;
        default: return;
        }

        CGraphics::SetBlendMode(ERglBlendMode::Blend,
                                ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha,
                                ERglLogicOp::Clear);

        /* Begin tri-strip of verts in x100_verts */
        /* Vtx Color xb4_ */
        /* UVs x130_clampedUVs[0], x130_clampedUVs[3] */
        /* UVs x130_clampedUVs[2], x130_clampedUVs[1] */
        /* UVs x130_clampedUVs[4], x130_clampedUVs[7] */
        /* UVs x130_clampedUVs[6], x130_clampedUVs[5] */
    }
    CGuiWidget::Draw(parms);
}

std::vector<ResId> CGuiStaticImage::GetTextureAssets() const
{
    std::vector<ResId> ret;
    ret.reserve(2);
    if ((x120_textureID1 & 0xffff) != 0xffff)
        ret.push_back(x120_textureID1);
    if ((x124_textureID2 & 0xffff) != 0xffff)
        ret.push_back(x124_textureID2);
    return ret;
}

CGuiStaticImage* CGuiStaticImage::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    float xDim = in.readFloatBig();
    float zDim = in.readFloatBig();
    zeus::CVector3f scaleCenter;
    scaleCenter.readBig(in);

    CGuiStaticImage::EMaterialType matType = CGuiStaticImage::EMaterialType(in.readUint32Big());
    ResId txtr1 = in.readUint32Big();
    ResId txtr2 = in.readUint32Big();

    EGuiTextureClampModeHorz clampH = EGuiTextureClampModeHorz(in.readUint32Big());
    EGuiTextureClampModeVert clampV = EGuiTextureClampModeVert(in.readUint32Big());

    float a = in.readFloatBig();
    float b = in.readFloatBig();
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    std::vector<float> floats = {a, b, a, d, c, b, c, d};

    CGuiStaticImage* ret = new CGuiStaticImage(parms, xDim, zDim, scaleCenter, clampH, clampV,
                                               matType, txtr1, txtr2, floats, true);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
