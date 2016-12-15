#include "CGuiModel.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, ResId modelId, u32 lightMask, bool flag)
: CGuiWidget(parms), x108_modelId(modelId), x10c_lightMask(lightMask)
{
    if (!flag || (modelId & 0xffff) == 0xffff ||
        parms.x0_frame->GetGuiSys().GetUsageMode() == CGuiSys::EUsageMode::Two)
        return;

    xf8_model = parms.x0_frame->GetGuiSys().GetResStore().GetObj({SBIG('CMDL'), modelId});
}

std::vector<ResId> CGuiModel::GetModelAssets() const
{
    return {x108_modelId};
}

bool CGuiModel::GetIsFinishedLoadingWidgetSpecific() const
{
    if (!xf8_model)
        return true;
    const CModel* model = xf8_model.GetObj();
    if (!model)
        return false;
    model->GetInstance().Touch(0);
    return model->IsLoaded(0);
}

void CGuiModel::Touch() const
{
    const CModel* model = xf8_model.GetObj();
    if (model)
        model->GetInstance().Touch(0);
}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) const
{
    CGraphics::SetModelMatrix(x34_worldXF);
    if (!xf8_model)
        return;
    if (!GetIsFinishedLoading())
        return;
    const CModel* model = xf8_model.GetObj();
    if (!model)
        return;

    if (GetIsVisible())
    {
        zeus::CColor moduCol = xa8_color2;
        moduCol.a *= parms.x0_alphaMod;
        xb0_frame->EnableLights(x10c_lightMask);
        if (xb6_29_cullFaces)
            CGraphics::SetCullMode(ERglCullMode::Front);

        switch (xac_drawFlags)
        {
        case EGuiModelDrawFlags::Shadeless:
        {
            CModelFlags flags(0, 0, 3, zeus::CColor::skWhite);
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Opaque:
        {
            CModelFlags flags(1, 0, 3, moduCol);
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Alpha:
        {
            CModelFlags flags(4, 0, (xb7_24_depthWrite << 1) | xb6_31_depthTest, moduCol);
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Additive:
        {
            CModelFlags flags(3, 0, (xb7_24_depthWrite << 1) | xb6_31_depthTest, moduCol);
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::AlphaAdditiveOverdraw:
        {
            CModelFlags flags(4, 0, xb6_31_depthTest, moduCol);
            model->Draw(flags);

            flags.m_blendMode = 5;
            flags.m_matSetIdx = 0;
            flags.m_flags = (xb7_24_depthWrite << 1) | xb6_31_depthTest;
            flags.color = moduCol;
            model->Draw(flags);
            break;
        }
        default: break;
        }

        if (xb6_29_cullFaces)
            CGraphics::SetCullMode(ERglCullMode::None);
        xb0_frame->DisableLights();
    }

    CGuiWidget::Draw(parms);
}

CGuiModel* CGuiModel::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    ResId model = in.readUint32Big();
    in.readUint32Big();
    u32 lightMask = in.readUint32Big();

    CGuiModel* ret = new CGuiModel(parms, model, lightMask, flag);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
