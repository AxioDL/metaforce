#pragma once

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
