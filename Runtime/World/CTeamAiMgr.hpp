#pragma once

#include "CEntity.hpp"

namespace urde
{

class CTeamAiData
{
    u32 x0_;
    u32 x4_;
    u32 x8_;
    u32 xc_;
    u32 x10_;
    u32 x14_;
    u32 x18_;
    float x1c_;
    float x20_;
public:
    CTeamAiData(CInputStream& in, s32 propCount);
};

class CTeamAiMgr : public CEntity
{
public:
    CTeamAiMgr(TUniqueId, std::string_view name, const CEntityInfo&, const CTeamAiData& data);

    void Accept(IVisitor&);
};
}

