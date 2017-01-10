#include "CGuiSliderGroup.hpp"

namespace urde
{

CGuiSliderGroup::CGuiSliderGroup(const CGuiWidgetParms& parms, float a, float b, float c, float d)
: CGuiCompoundWidget(parms), xf8_minVal(a), xfc_maxVal(b), x100_curVal(c), x104_increment(d)
{
}

void CGuiSliderGroup::SetSelectionChangedCallback
    (std::function<void(CGuiSliderGroup*, float)>&& func)
{
    x114_changeCallback = std::move(func);
}

void CGuiSliderGroup::SetCurVal(float cur)
{
    x100_curVal = zeus::clamp(xf8_minVal, cur, xfc_maxVal);
    float factor = 0.f;
    if (xfc_maxVal != xf8_minVal)
        factor = (x100_curVal - xf8_minVal) / (xfc_maxVal - xf8_minVal);

    const zeus::CVector3f& w0Idle = x10c_workers[0]->GetIdlePosition();
    const zeus::CVector3f& w1Idle = x10c_workers[1]->GetIdlePosition();
    x10c_workers[0]->SetLocalPosition(zeus::CVector3f::lerp(w0Idle, w1Idle, factor));
}

bool CGuiSliderGroup::AddWorkerWidget(CGuiWidget* worker)
{
    if (worker->GetWorkerId() < 0 || worker->GetWorkerId() > 1)
        return true;
    x10c_workers[worker->GetWorkerId()] = worker;
    return true;
}

CGuiWidget* CGuiSliderGroup::GetWorkerWidget(int id)
{
    if (id < 0 || id > 1)
        return nullptr;
    return x10c_workers[id];
}

CGuiSliderGroup* CGuiSliderGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    float min = in.readFloatBig();
    float max = in.readFloatBig();
    float cur = in.readFloatBig();
    float increment = in.readFloatBig();

    CGuiSliderGroup* ret = new CGuiSliderGroup(parms, min, max, cur, increment);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
