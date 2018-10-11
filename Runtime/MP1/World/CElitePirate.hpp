#pragma once

#include "World/CPatterned.hpp"
#include "World/CActorParameters.hpp"
#include "World/CAnimationParameters.hpp"

namespace urde::MP1
{
class CElitePirateData
{
    float x0_;
    float x4_;
    float x8_;
    float xc_;
    float x10_;
    float x14_;
    float x18_;
    float x1c_;
    CAssetId x20_;
    s16 x24_;
    CActorParameters x28_;
    CAnimationParameters x90_;
    CAssetId x9c_;
    s16 xa0_;
    CAssetId xa4_;
    CDamageInfo xa8_;
    float xc4_;
    CAssetId xc8_;
    CAssetId xcc_;
    CAssetId xd0_;
    CAssetId xd4_;
    float xd8_;
    float xdc_;
    float xe0_;
    float xe4_;
    float xe8_;
    float xec_;
    u32 xf0_;
    u32 xf4_;
    CAssetId xf8_;
    CDamageInfo xfc_;
    CAssetId x118_;
    s16 x11c_;
    bool x11e_;
    bool x11f_;
public:
    CElitePirateData(CInputStream&, u32 propCount);
};

class CElitePirate : public CPatterned
{
public:
    DEFINE_PATTERNED(ElitePirate)

    CElitePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CPatternedInfo&, const CActorParameters&, const CElitePirateData&);
};
}