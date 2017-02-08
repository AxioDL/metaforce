#include "CSaveWorld.hpp"
#include "CToken.hpp"

namespace urde
{
CSaveWorld::CSaveWorld(CInputStream &in)
{
    in.readUint32Big();
    u32 version = in.readUint32Big();
    if (version > 1)
        x0_areaCount = in.readUint32Big();
    if (version > 2)
    {
        u32 cinematicCount = in.readUint32Big();
        x4_cinematics.reserve(cinematicCount);
        for (u32 i=0 ; i<cinematicCount ; ++i)
            x4_cinematics.push_back(in.readUint32Big());

        u32 relayCount = in.readUint32Big();
        x14_relays.reserve(relayCount);
        for (u32 i=0 ; i<relayCount ; ++i)
            x14_relays.push_back(in.readUint32Big());
    }

    u32 layerCount = in.readUint32Big();
    x24_layers.reserve(layerCount);
    for (u32 i=0 ; i<layerCount ; ++i)
    {
        x24_layers.emplace_back();
        SLayerState& st = x24_layers.back();
        st.x0_area = in.readUint32Big();
        st.x4_layer = in.readUint32Big();
    }

    u32 doorCount = in.readUint32Big();
    x34_doors.reserve(doorCount);
    for (u32 i=0 ; i<doorCount ; ++i)
        x34_doors.push_back(in.readUint32Big());
    if (version > 0)
    {
        u32 scanCount = in.readUint32Big();
        x44_scans.reserve(scanCount);
        for (u32 i=0 ; i<scanCount ; ++i)
        {
            x44_scans.emplace_back();
            SScanState& st = x44_scans.back();
            st.x0_id = in.readUint32Big();
            st.x4_category = EScanCategory(in.readUint32Big());
        }
    }
}

u32 CSaveWorld::GetAreaCount() const
{
    return x0_areaCount;
}

u32 CSaveWorld::GetCinematicCount() const
{
    return x4_cinematics.size();
}

s32 CSaveWorld::GetCinematicIndex(const TEditorId &id) const
{
    auto it = std::find(x4_cinematics.begin(), x4_cinematics.end(), id);
    if (it == x4_cinematics.end())
        return -1;
    return it - x4_cinematics.begin();
}

u32 CSaveWorld::GetRelayCount() const
{
    return x14_relays.size();
}

s32 CSaveWorld::GetRelayIndex(const TEditorId &id) const
{
    auto it = std::find(x14_relays.begin(), x14_relays.end(), id);
    if (it == x14_relays.end())
        return -1;
    return it - x14_relays.begin();
}

TEditorId CSaveWorld::GetRelayEditorId(u32 idx) const
{
    return x14_relays[idx];
}

u32 CSaveWorld::GetDoorCount() const
{
    return x34_doors.size();
}

s32 CSaveWorld::GetDoorIndex(const TEditorId &id) const
{
    auto it = std::find(x34_doors.begin(), x34_doors.end(), id);
    if (it == x34_doors.end())
        return -1;
    return it - x34_doors.begin();
}

CFactoryFnReturn FSaveWorldFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                   CObjectReference* selfRef)
{
    return TToken<CSaveWorld>::GetIObjObjectFor(std::make_unique<CSaveWorld>(in));
}

}
