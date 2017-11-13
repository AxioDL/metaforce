#ifndef __URDE_CGUIWIDGETIDDB_HPP__
#define __URDE_CGUIWIDGETIDDB_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"

namespace urde
{

class CGuiWidgetIdDB
{
    std::unordered_map<std::string, s16> x0_dbMap;
    s16 x14_lastPoolId = 0;
public:
    CGuiWidgetIdDB();
    s16 FindWidgetID(std::string_view name) const;
    s16 AddWidget(std::string_view name, s16 id);
    s16 AddWidget(std::string_view name);
};

}

#endif // __URDE_CGUIWIDGETIDDB_HPP__
