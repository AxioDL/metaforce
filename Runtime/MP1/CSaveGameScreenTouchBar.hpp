#ifndef __URDE_CSAVEUITOUCHBAR_HPP__
#define __URDE_CSAVEUITOUCHBAR_HPP__

#include <utility>
#include <memory>
#include <string>

namespace urde
{
namespace MP1
{

class CSaveGameScreenTouchBar
{
public:
    virtual ~CSaveGameScreenTouchBar() = default;
    virtual int PopOption();
    virtual void SetUIOpts(std::u16string_view opt0,
                           std::u16string_view opt1,
                           std::u16string_view opt2);
};

std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar();

}
}

#endif // __URDE_CSAVEUITOUCHBAR_HPP__
