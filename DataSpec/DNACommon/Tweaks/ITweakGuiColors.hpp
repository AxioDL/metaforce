#ifndef __DNACOMMON_ITWEAKGUICOLORS_HPP__
#define __DNACOMMON_ITWEAKGUICOLORS_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{
struct ITweakGuiColors : BigYAML
{
    virtual zeus::CColor GetHudMessageFill() const=0;
    virtual zeus::CColor GetHudMessageOutline() const=0;
    virtual zeus::CColor GetHudFrameColor() const=0;
    virtual zeus::CColor GetTickDecoColor() const=0;
    virtual zeus::CColor GetHudCounterFill() const=0;
    virtual zeus::CColor GetHudCounterOutline() const=0;
};
}

#endif // __DNACOMMON_ITWEAKGUICOLORS_HPP__
