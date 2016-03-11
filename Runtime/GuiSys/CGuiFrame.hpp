#ifndef __URDE_CGUIFRAME_HPP__
#define __URDE_CGUIFRAME_HPP__

#include "CGuiWidget.hpp"

namespace urde
{
class CGuiSys;

enum class EFrameTransitionOptions
{
    Zero
};

class CGuiFrameTransitionOptions
{
    EFrameTransitionOptions x0_opts;
    bool x4_ = true;
    float x8_ = 1.f;
    zeus::CVector3f xc_;
public:
    CGuiFrameTransitionOptions(EFrameTransitionOptions opts)
    : x0_opts(opts) {}
};

class CGuiFrame
{
    std::string x4_name;
    u32 x14_id;
    CGuiFrameTransitionOptions x1c_transitionOpts;
public:
    CGuiFrame(u32 id, const std::string& name, CGuiSys& sys, int, int, int);
};

}

#endif // __URDE_CGUIFRAME_HPP__
