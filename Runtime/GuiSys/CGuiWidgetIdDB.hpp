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
    s16 FindWidgetID(const std::string& name) const;
    s16 AddWidget(const std::string& name, s16 id);
    s16 AddWidget(const std::string& name);
};

}

#endif // __URDE_CGUIWIDGETIDDB_HPP__
