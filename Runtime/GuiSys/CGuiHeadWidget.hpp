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
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    std::shared_ptr<CGuiHeadWidget> shared_from_this()
    { return std::static_pointer_cast<CGuiHeadWidget>(CGuiObject::shared_from_this()); }
};

}

#endif // __URDE_CGUIHEADWIDGET_HPP__
