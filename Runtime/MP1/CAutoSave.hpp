#ifndef __URDE_CAUTOSAVE_HPP__
#define __URDE_CAUTOSAVE_HPP__

#include "CIOWin.hpp"

namespace urde::MP1
{
class CSaveGameScreen;
class CAutoSave : CIOWin
{
    std::unique_ptr<CSaveGameScreen> x14_savegameScreen;
public:
    CAutoSave();
};
}
#endif // __URDE_CAUTOSAVE_HPP__
