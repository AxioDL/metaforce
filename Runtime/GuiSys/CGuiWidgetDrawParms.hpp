#ifndef __URDE_CGUIWIDGETDRAWPARMS_HPP__
#define __URDE_CGUIWIDGETDRAWPARMS_HPP__

namespace urde
{

struct CGuiWidgetDrawParms
{
    float x0_alphaMod = 1.f;
    float x4_ = 0.f;
    float x8_ = 0.f;
    float xc_ = 0.f;

    static CGuiWidgetDrawParms Default;
};

}

#endif // __URDE_CGUIWIDGETDRAWPARMS_HPP__
