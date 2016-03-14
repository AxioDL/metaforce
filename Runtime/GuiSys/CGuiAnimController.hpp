#ifndef __URDE_CGUIANIMCONTROLLER_HPP__
#define __URDE_CGUIANIMCONTROLLER_HPP__

#include "CGuiWidget.hpp"
#include <array>

namespace urde
{

class CGuiAnimSet
{
};

class CGuiAnimController : public CGuiWidget
{
    std::array<std::unique_ptr<CGuiAnimSet>, 13> xf8_sets;
    float x164_ = 0.f;
    CGuiWidget* x168_widget;
    u32 x16c_ = 0;
    u32 x170_ = 0;

public:
    CGuiAnimController(const CGuiWidget::CGuiWidgetParms& parms, CGuiWidget* widget);
};

}

#endif // __URDE_CGUIANIMCONTROLLER_HPP__
