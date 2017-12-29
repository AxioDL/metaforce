#include "CSaveGameScreenTouchBar.hpp"

namespace urde::MP1
{

int CSaveGameScreenTouchBar::PopOption() { return -1; }
void CSaveGameScreenTouchBar::SetUIOpts(std::u16string_view opt0,
                                        std::u16string_view opt1,
                                        std::u16string_view opt2) {}

#ifndef __APPLE__
std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar()
{
    return std::make_unique<CSaveGameScreenTouchBar>();
}
#endif

}
