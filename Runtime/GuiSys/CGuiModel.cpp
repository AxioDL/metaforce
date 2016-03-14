#include "CGuiModel.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "CGuiAnimController.hpp"

namespace urde
{

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, TResId modelId, u32 lightMode, bool flag)
: CGuiWidget(parms), x108_modelId(modelId), x10c_lightMode(lightMode)
{
    if (!flag || (modelId & 0xffff) == 0xffff ||
        parms.x0_frame->GetGuiSys().GetUsageMode() == CGuiSys::EUsageMode::Two)
        return;

    xf8_model = parms.x0_frame->GetGuiSys().GetResStore().GetObj({SBIG('CMDL'), modelId});
}

CGuiModel* CGuiModel::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    TResId model = in.readUint32Big();
    in.readUint32Big();
    u32 lightMode = in.readUint32Big();

    CGuiModel* ret = new CGuiModel(parms, model, lightMode, flag);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
