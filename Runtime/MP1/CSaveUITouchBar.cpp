#include "CSaveUITouchBar.hpp"

namespace urde
{
namespace MP1
{

int CSaveUITouchBar::PopOption() { return -1; }
void CSaveUITouchBar::SetUIOpts(const std::u16string& opt0,
                                const std::u16string& opt1,
                                const std::u16string& opt2) {}

#ifndef __APPLE__
std::unique_ptr<CSaveUITouchBar> NewSaveUITouchBar()
{
    return std::make_unique<CSaveUITouchBar>();
}
#endif

}
}
