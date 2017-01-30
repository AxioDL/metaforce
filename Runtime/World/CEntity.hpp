#ifndef __URDE_CENTITY_HPP__
#define __URDE_CENTITY_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "CEntityInfo.hpp"

namespace urde
{
class CStateManager;
class IVisitor;

class CEntity
{
    friend class CStateManager;
    friend class CObjectList;
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
            bool x30_25_inGraveyard : 1;
            bool x30_26_scriptingBlocked : 1;
            bool x30_27_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    static const std::vector<SConnection> NullConnectionList;
    virtual ~CEntity() = default;
    CEntity(TUniqueId uid, const CEntityInfo& info, bool active, const std::string& name);
    virtual void Accept(IVisitor& visitor)=0;
    virtual void PreThink(float, CStateManager&) {}
    virtual void Think(float, CStateManager&) {}
    virtual void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    virtual void SetActive(bool active) {x30_24_active = active;}

    bool GetActive() const {return x30_24_active;}
    void ToggleActive()
    {
        x30_24_active ^= 1;
    }

    bool IsInGraveyard() const { return x30_25_inGraveyard; }
    void SetIsInGraveyard(bool in) { x30_25_inGraveyard = in; }
    bool IsScriptingBlocked() const { return x30_26_scriptingBlocked; }
    void SetIsScriptingBlocked(bool blocked) { x30_26_scriptingBlocked = blocked; }

    TAreaId GetAreaId() const
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
