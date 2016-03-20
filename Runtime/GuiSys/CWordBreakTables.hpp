#ifndef __URDE_CWORDBREAKTABLES_HPP__
#define __URDE_CWORDBREAKTABLES_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CWordBreakTables
{
public:
    static int GetBeginRank(wchar_t ch);
    static int GetEndRank(wchar_t ch);
};

}

#endif // __URDE_CWORDBREAKTABLES_HPP__
