#ifndef __URDE_CGUISLIDERGROUP_HPP__
#define __URDE_CGUISLIDERGROUP_HPP__

#include "CGuiCompoundWidget.hpp"
#include <functional>

namespace urde
{

class CGuiSliderGroup : public CGuiCompoundWidget
{
    float xf8_minVal;
    float xfc_maxVal;
    float x100_curVal;
    float x104_increment;
    u32 x108_ = 2;
    CGuiWidget* x10c_workers[2] = {};
    std::function<void(const CGuiSliderGroup*, float)> x114_changeCallback;
public:
    CGuiSliderGroup(const CGuiWidgetParms& parms, float a, float b, float c, float d);
    FourCC GetWidgetTypeID() const {return FOURCC('SLGP');}

    void SetSelectionChangedCallback(std::function<void(const CGuiSliderGroup*, float)>&& func);
    void SetIncrement(float inc) {x104_increment = inc;}
    void SetMinVal(float min) {xf8_minVal = min; SetCurVal(x100_curVal);}
    void SetMaxVal(float max) {xfc_maxVal = max; SetCurVal(x100_curVal);}
    void SetCurVal(float cur);

    bool AddWorkerWidget(CGuiWidget* worker);
    CGuiWidget* GetWorkerWidget(int id);

    static CGuiSliderGroup* Create(CGuiFrame* frame, CInputStream& in, bool flag);
};

}

#endif // __URDE_CGUISLIDERGROUP_HPP__
