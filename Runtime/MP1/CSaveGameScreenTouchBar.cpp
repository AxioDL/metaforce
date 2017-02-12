#include "CSaveGameScreenTouchBar.hpp"

namespace urde
{
namespace MP1
{

int CSaveGameScreenTouchBar::PopOption() { return -1; }
void CSaveGameScreenTouchBar::SetUIOpts(const std::u16string& opt0,
                                        const std::u16string& opt1,
                                        const std::u16string& opt2) {}

#ifndef __APPLE__
std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar()
{
    return std::make_unique<CSaveGameScreenTouchBar>();
}
#endif

}
}
