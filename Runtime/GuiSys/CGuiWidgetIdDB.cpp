#include "CGuiWidgetIdDB.hpp"

namespace urde
{

CGuiWidgetIdDB::CGuiWidgetIdDB()
{
    AddWidget("kGSYS_DummyWidgetID", 0);
    AddWidget("kGSYS_HeadWidgetID", 1);
    AddWidget("kGSYS_DefaultCameraID");
    AddWidget("kGSYS_DefaultLightID");
}

s16 CGuiWidgetIdDB::FindWidgetID(const std::string& name) const
{
    auto search = x0_dbMap.find(name);
    if (search == x0_dbMap.cend())
        return -1;
    return search->second;
}

s16 CGuiWidgetIdDB::AddWidget(const std::string& name, s16 id)
{
    s16 findId = FindWidgetID(name);
    if (findId == -1)
    {
        if (id >= x14_lastPoolId)
            x14_lastPoolId = id;
        x0_dbMap.emplace(std::make_pair(name, id));
        findId = id;
    }
    return findId;
}

s16 CGuiWidgetIdDB::AddWidget(const std::string& name)
{
    s16 findId = FindWidgetID(name);
    if (findId == -1)
    {
        ++x14_lastPoolId;
        x0_dbMap.emplace(std::make_pair(name, x14_lastPoolId));
        findId = x14_lastPoolId;
    }
    return findId;
}

}
