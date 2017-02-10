#ifndef __URDE_CGAMEOPTIONSTOUCHBAR_HPP__
#define __URDE_CGAMEOPTIONSTOUCHBAR_HPP__

#include <utility>
#include <memory>

namespace urde
{

class CGameOptionsTouchBar
{
public:
    enum class EAction
    {
        None,
        Back,
        Advance,
        ValueChange
    };

    virtual ~CGameOptionsTouchBar() = default;
    virtual EAction PopAction();
    virtual void GetSelection(int& left, int& right, int& value);
    virtual void SetSelection(int left, int right, int value);
};

std::unique_ptr<CGameOptionsTouchBar> NewGameOptionsTouchBar();

}

#endif // __URDE_CGAMEOPTIONSTOUCHBAR_HPP__
