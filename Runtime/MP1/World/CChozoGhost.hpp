#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CChozoGhost : public CPatterned
{
public:
    class CBehaveChance
    {
        u32 x0_propertyCount;
        float x4_;
        float x8_;
        float xc_;
        float x10_;
        float x14_;
        float x18_;
        u32 x1c_;
    public:
        CBehaveChance(CInputStream&);
    };

private:
public:
    DEFINE_PATTERNED(ChozoGhost)

    CChozoGhost(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                const CActorParameters&, const CPatternedInfo&, float, float, float, float, CAssetId,
                const CDamageInfo&, CAssetId, const CDamageInfo&, const CChozoGhost::CBehaveChance&,
                const CChozoGhost::CBehaveChance&, const CBehaveChance&, u16, float, u16, u16, u32, float, u32, float,
                CAssetId, s16, float, float, u32, u32);
};
}