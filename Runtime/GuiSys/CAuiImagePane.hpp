#ifndef __URDE_CAUIIMAGEPANE_HPP__
#define __URDE_CAUIIMAGEPANE_HPP__

#include "CGuiWidget.hpp"

namespace urde
{
class CSimplePool;

class CAuiImagePane : public CGuiWidget
{
public:
    CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, ResId, ResId,
                  rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                  rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool);
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};
}

#endif // __URDE_CAUIIMAGEPANE_HPP__
