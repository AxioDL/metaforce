#ifndef __PSHAG_CENTITY_HPP__
#define __PSHAG_CENTITY_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"

namespace pshag
{
class CStateManager;

struct SConnection
{
    EScriptObjectState state;
    EScriptObjectMessage msg;
    TEditorId objId;
};

class CEntityInfo
{
    friend class CEntity;
    TAreaId m_aid;
    std::vector<SConnection> m_conns;
public:
    CEntityInfo(TAreaId aid, const std::vector<SConnection>& conns)
    : m_aid(aid), m_conns(conns) {}
};

class CEntity
{
protected:
    TUniqueId m_uid;
    CEntityInfo m_info;
    bool m_active = false;
public:
    virtual ~CEntity() {}
    CEntity(TUniqueId uid, const CEntityInfo& info, bool active);
    virtual void PreThink(float, CStateManager&) {}
    virtual void Think(float, CStateManager&) {}
    virtual void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    virtual bool GetActive() const {return m_active;}
    virtual void SetActive(bool active) {m_active = active;}

    void SendScriptMsgs(EScriptObjectState state, CStateManager& stateMgr, EScriptObjectMessage msg);
};

}

#endif // __PSHAG_CENTITY_HPP__
