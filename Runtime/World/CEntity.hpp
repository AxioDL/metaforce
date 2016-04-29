#ifndef __URDE_CENTITY_HPP__
#define __URDE_CENTITY_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"

namespace urde
{
class CStateManager;

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
    CEntityInfo(TAreaId aid, const std::vector<SConnection>& conns, ResId savwId=-1)
    : x0_areaId(aid), x4_conns(conns) {}
    TAreaId GetAreaId() const {return x0_areaId;}
    std::vector<SConnection> GetConnectionList() const { return x4_conns; }
    ResId GetEditorId() const { return x14_editorId; }
};

class CEntity
{
protected:
    TAreaId x4_areaId;
    TUniqueId x8_uid;
    TEditorId xc_editorId;
    std::string x10_name;
    std::vector<SConnection> x20_conns;

    union
    {
        struct
        {
            bool x30_24_active : 1;
            bool x30_25_ : 1;
            bool x30_26_ : 1;
            bool x30_27_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    static const std::vector<SConnection> NullConnectionList;
    virtual ~CEntity() {}
    CEntity(TUniqueId uid, const CEntityInfo& info, bool active, const std::string& name);
    virtual void PreThink(float, CStateManager&) {}
    virtual void Think(float, CStateManager&) {}
    virtual void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    bool GetActive() const {return x30_24_active;}
    virtual void SetActive(bool active) {x30_24_active = active;}
    void ToggleActive()
    {
        x30_24_active ^= 1;
    }

    TAreaId GetCurrentAreaId() const
    {
        if (x30_27_)
            return x4_areaId;
        return kInvalidAreaId;
    }
    TUniqueId GetUniqueId() const {return x8_uid;}
    void SendScriptMsgs(EScriptObjectState state, CStateManager& stateMgr, EScriptObjectMessage msg);
};

}

#endif // __URDE_CENTITY_HPP__
