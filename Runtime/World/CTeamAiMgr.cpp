#include "CTeamAiMgr.hpp"
#include "TCastTo.hpp"
namespace urde
{

CTeamAiData::CTeamAiData(CInputStream& in, s32 propCount)
: x0_(in.readUint32Big())
, x4_(in.readUint32Big())
, x8_(in.readUint32Big())
, xc_(in.readUint32Big())
, x10_(in.readUint32Big())
, x14_(in.readUint32Big())
, x18_(in.readUint32Big())
, x1c_(propCount > 8 ? in.readFloatBig() : 0.f)
, x20_(propCount > 8 ? in.readFloatBig() : 0.f)
{
}

CTeamAiMgr::CTeamAiMgr(TUniqueId uid, std::string_view name, const CEntityInfo& info, const CTeamAiData& data)
    : CEntity(uid, info, true, name)
{
}

void CTeamAiMgr::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}
}
