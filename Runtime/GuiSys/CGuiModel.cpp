#include "CGuiModel.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, TResId modelId, u32 lightMask, bool flag)
: CGuiWidget(parms), x108_modelId(modelId), x10c_lightMask(lightMask)
{
    if (!flag || (modelId & 0xffff) == 0xffff ||
        parms.x0_frame->GetGuiSys().GetUsageMode() == CGuiSys::EUsageMode::Two)
        return;

    xf8_model = parms.x0_frame->GetGuiSys().GetResStore().GetObj({SBIG('CMDL'), modelId});
}

std::vector<TResId> CGuiModel::GetModelAssets() const
{
    return {x108_modelId};
}

bool CGuiModel::GetIsFinishedLoadingWidgetSpecific() const
{
    if (!xf8_model)
        return true;
    CModel* model = xf8_model.GetObj();
    if (!model)
        return false;
    model->Touch(0);
    return model->IsLoaded(0);
}

void CGuiModel::Touch() const
{
    CModel* model = xf8_model.GetObj();
    if (model)
        model->Touch(0);
}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) const
{
    CGraphics::SetModelMatrix(x34_worldXF);
    if (!xf8_model)
        return;
    if (!GetIsFinishedLoading())
        return;
    CModel* model = xf8_model.GetObj();
    if (!model)
        return;

    if (GetIsVisible())
    {
        zeus::CColor moduCol = xb4_;
        moduCol.a *= parms.x0_alphaMod;
        xc8_frame->EnableLights(x10c_lightMask);
        if (xf6_29_cullFaces)
            CGraphics::SetCullMode(ERglCullMode::Front);

        switch (xc4_drawFlags)
        {
        case EGuiModelDrawFlags::Zero:
        {
            CModelFlags flags;
            flags.f1 = 0;
            flags.f2 = 0;
            flags.f3 = 3;
            flags.color = zeus::CColor::skWhite;
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::One:
        {
            CModelFlags flags;
            flags.f1 = 1;
            flags.f2 = 0;
            flags.f3 = 3;
            flags.color = moduCol;
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Two:
        {
            CModelFlags flags;
            flags.f1 = 4;
            flags.f2 = 0;
            flags.f3 = (xf7_24_ << 1) | xf6_31_;
            flags.color = moduCol;
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Three:
        {
            CModelFlags flags;
            flags.f1 = 3;
            flags.f2 = 0;
            flags.f3 = (xf7_24_ << 1) | xf6_31_;
            flags.color = moduCol;
            model->Draw(flags);
            break;
        }
        case EGuiModelDrawFlags::Four:
        {
            CModelFlags flags;
            flags.f1 = 4;
            flags.f2 = 0;
            flags.f3 = xf6_31_;
            flags.color = moduCol;
            model->Draw(flags);

            flags.f1 = 5;
            flags.f2 = 0;
            flags.f3 = (xf7_24_ << 1) | xf6_31_;
            flags.color = moduCol;
            model->Draw(flags);
            break;
        }
        default: break;
        }

        if (xf6_29_cullFaces)
            CGraphics::SetCullMode(ERglCullMode::None);
    }

    CGuiWidget::Draw(parms);
}

CGuiModel* CGuiModel::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    TResId model = in.readUint32Big();
    in.readUint32Big();
    u32 lightMask = in.readUint32Big();

    CGuiModel* ret = new CGuiModel(parms, model, lightMask, flag);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
