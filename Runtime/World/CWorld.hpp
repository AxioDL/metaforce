#ifndef __URDE_CWORLD_HPP__
#define __URDE_CWORLD_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"

namespace urde
{
class CGameArea;
class IObjectStore;
class CResFactory;

class CWorld
{
    ResId xc_worldId = -1;
    ResId x10_ = -1;
    ResId x24_ = -1;

    std::vector<std::unique_ptr<CGameArea>> x18_areas;

    std::unique_ptr<u8[]> x40_;
    std::unique_ptr<u8[]> x44_;

    IObjectStore* x60_objectStore;
    CResFactory* x64_resFactory;

    union
    {
        struct
        {
            bool x70_24_ : 1;
            bool x70_25_ : 1;
            bool x70_26_ : 1;
        };
    };
public:

    class CRelay
    {
        TEditorId x0_relay = kInvalidEditorId;
        TEditorId x4_target = kInvalidEditorId;
        s16 x8_msg = -1;
        bool xa_active = false;
    public:
        CRelay() = default;
        CRelay(CInputStream& in);

        TEditorId GetRelayId() const { return x0_relay; }
        TEditorId GetTargetId() const { return x4_target; }
        s16 GetMessage() const { return x8_msg; }
        bool GetActive() const { return xa_active; }
    };

    CWorld(IObjectStore& objStore, CResFactory& resFactory, ResId);
    bool DoesAreaExist(TAreaId area) const;
    std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() {return x18_areas;}
};

}

#endif // __URDE_CWORLD_HPP__

