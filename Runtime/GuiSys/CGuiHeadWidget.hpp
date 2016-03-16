#ifndef __URDE_CGUIHEADWIDGET_HPP__
#define __URDE_CGUIHEADWIDGET_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CGuiHeadWidget : public CGuiWidget
{
public:
    FourCC GetWidgetTypeID() const {return FOURCC('HWIG');}
    CGuiHeadWidget(const CGuiWidgetParms& parms);
    static CGuiHeadWidget* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUIHEADWIDGET_HPP__
