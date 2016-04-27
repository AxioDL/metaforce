#include "CPASAnimState.hpp"

namespace urde
{

CPASAnimState::CPASAnimState(CInputStream& in)
{
    x0_id = in.readUint32Big();
    u32 parmCount = in.readUint32Big();
    u32 animCount = in.readUint32Big();

    x4_parms.reserve(parmCount);
    x14_anims.reserve(animCount);
    x24_.reserve(animCount);

    for (u32 i=0 ; i<parmCount ; ++i)
        x4_parms.emplace_back(in);

    for (u32 i=0 ; i<animCount ; ++i)
    {
        s32 id = in.readUint32Big();
        rstl::reserved_vector<CPASAnimParm::UParmValue, 8> parms;
        for (const CPASParmInfo& parm : x4_parms)
        {
            CPASAnimParm::UParmValue val = {};
            switch (parm.GetType())
            {
            case CPASAnimParm::EParmType::Int32:
                val.m_int = in.readInt32Big();
                break;
            case CPASAnimParm::EParmType::UInt32:
                val.m_uint = in.readUint32Big();
                break;
            case CPASAnimParm::EParmType::Float:
                val.m_float = in.readFloatBig();
                break;
            case CPASAnimParm::EParmType::Bool:
                val.m_bool = in.readBool();
                break;
            case CPASAnimParm::EParmType::Enum:
                val.m_int = in.readInt32Big();
                break;
            default: break;
            }
            parms.push_back(val);
        }

        auto search = std::lower_bound(x14_anims.begin(), x14_anims.end(), id,
        [](const CPASAnimInfo& item, const u32& testId) -> bool {return item.GetAnimId() < testId;});
        x14_anims.emplace(search, id, std::move(parms));
    }
}

std::pair<float, s32> CPASAnimState::FindBestAnimation(const rstl::reserved_vector<CPASAnimParm, 8>&, CRandom16&, s32) const
{
}
}
