#ifndef __URDE_CENTITYINFO_HPP__
#define __URDE_CENTITYINFO_HPP__

#include <vector>
#include <string>
#include "RetroTypes.hpp"
#include "World/ScriptObjectSupport.hpp"

namespace urde
{
struct SConnection
{
    EScriptObjectState x0_state;
    EScriptObjectMessage x4_msg;
    TEditorId x8_objId;
};

class CEntityInfo
{
    TAreaId x0_areaId;
    std::vector<SConnection> x4_conns;
    TEditorId x14_editorId;
public:
    CEntityInfo(TAreaId aid, const std::vector<SConnection>& conns, TEditorId eid = kInvalidEditorId)
    : x0_areaId(aid), x4_conns(conns), x14_editorId(eid) {}
    TAreaId GetAreaId() const {return x0_areaId;}
    std::vector<SConnection> GetConnectionList() const { return x4_conns; }
    TEditorId GetEditorId() const { return x14_editorId; }
};
}

#endif // __URDE_CENTITYINFO_HPP__
