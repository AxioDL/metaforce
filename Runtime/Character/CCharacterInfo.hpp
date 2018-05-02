#ifndef __URDE_CCHARACTERINFO_HPP__
#define __URDE_CCHARACTERINFO_HPP__

#include "IOStreams.hpp"
#include "CPASDatabase.hpp"
#include "zeus/CAABox.hpp"
#include "CEffectComponent.hpp"

namespace urde
{

class CCharacterInfo
{
    friend class CAnimData;

public:
    struct CParticleResData
    {
        std::vector<CAssetId> x0_part;
        std::vector<CAssetId> x10_swhc;
        std::vector<CAssetId> x20_elsc;
        std::vector<CAssetId> x30_elsc;
        CParticleResData(CInputStream& in, u16 tableCount);
    };

private:
    u16 x0_tableCount;
    std::string x4_name;
    CAssetId x14_cmdl;
    CAssetId x18_cskr;
    CAssetId x1c_cinf;
    std::vector<std::pair<u32, std::pair<std::string, std::string>>> x20_animInfo;
    CPASDatabase x30_pasDatabase;
    CParticleResData x44_partRes;
    u32 x84_unk;
    std::vector<std::pair<std::string, zeus::CAABox>> x88_aabbs;
    std::vector<std::pair<std::string, std::vector<CEffectComponent>>> x98_effects;

    CAssetId xa8_cmdlOverlay = 0;
    CAssetId xac_cskrOverlay = 0;

    std::vector<s32> xb0_animIdxs;

public:
    CCharacterInfo(CInputStream& in);

    std::string_view GetCharacterName() const { return x4_name; }
    CAssetId GetModelId() const { return x14_cmdl; }
    CAssetId GetSkinRulesId() const { return x18_cskr; }
    CAssetId GetCharLayoutInfoId() const { return x1c_cinf; }

    const std::vector<std::pair<std::string, zeus::CAABox>>& GetAnimBBoxList() const { return x88_aabbs; }
    const std::vector<std::pair<std::string, std::vector<CEffectComponent>>>& GetEffectList() const
    {
        return x98_effects;
    }

    CAssetId GetIceModelId() const { return xa8_cmdlOverlay; }
    CAssetId GetIceSkinRulesId() const { return xac_cskrOverlay; }

    const CParticleResData& GetParticleResData() const { return x44_partRes; }
    s32 GetAnimationIndex(s32 idx) const { return xb0_animIdxs.at(idx); }
    const CPASDatabase& GetPASDatabase() const { return x30_pasDatabase; }
};
}

#endif // __URDE_CCHARACTERINFO_HPP__
