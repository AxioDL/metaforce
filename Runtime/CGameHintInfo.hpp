#ifndef __URDE_CGAMEHINTINFO_HPP__
#define __URDE_CGAMEHINTINFO_HPP__

#include "RetroTypes.hpp"
#include "IFactory.hpp"

namespace urde
{
class CGameHintInfo
{
public:
    struct SHintLocation
    {
        ResId x0_mlvlId = -1;
        ResId x4_mreaId = -1;
        TAreaId x8_areaId = kInvalidAreaId;
        ResId xc_stringId = -1;
        SHintLocation(CInputStream&, s32);
    };

    class CGameHint
    {
        std::string x0_name;
        float x10_;
        float x14_fadeInTime;
        ResId x18_stringId;
        float x1c_;
        std::vector<SHintLocation> x20_locations;
    public:
        CGameHint(CInputStream&, s32);
    };

private:
    std::vector<CGameHint> x0_hints;
public:
    CGameHintInfo(CInputStream&, s32);
};

CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream&, const CVParamTransfer, CObjectReference*);
}
#endif // __URDE_CGAMEHINTINFO_HPP__
