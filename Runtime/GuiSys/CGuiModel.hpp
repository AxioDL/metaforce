#ifndef __URDE_CGUIMODEL_HPP__
#define __URDE_CGUIMODEL_HPP__

#include "CGuiWidget.hpp"
#include "CToken.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{

class CGuiModel : public CGuiWidget
{
    TLockedToken<CModel> xf8_model;
    TResId x108_modelId;
    u32 x10c_lightMode;
public:
    CGuiModel(const CGuiWidgetParms& parms, TResId modelId, u32 lightMode, bool flag);
    static CGuiModel* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUIMODEL_HPP__
