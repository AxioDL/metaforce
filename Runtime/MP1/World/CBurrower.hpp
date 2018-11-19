#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{

class CBurrower : public CPatterned
{
    float x6a4_ = 0.f;
public:
    DEFINE_PATTERNED(Burrower)
    CBurrower(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
        const CPatternedInfo&, const CActorParameters&, CAssetId, CAssetId, CAssetId, const CDamageInfo&, CAssetId, u32,
        CAssetId);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
};
}
