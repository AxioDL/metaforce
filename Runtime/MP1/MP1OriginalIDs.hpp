#pragma once

#include "RetroTypes.hpp"
#include "IFactory.hpp"

namespace urde
{

class MP1OriginalIDs
{
    std::vector<std::pair<CAssetId, CAssetId>> m_origToNew;
    std::vector<std::pair<CAssetId, CAssetId>> m_newToOrig;

public:
    MP1OriginalIDs(CInputStream& in);
    CAssetId TranslateOriginalToNew(CAssetId id) const;
    CAssetId TranslateNewToOriginal(CAssetId id) const;
};

CFactoryFnReturn FMP1OriginalIDsFactory(const SObjectTag& tag, CInputStream& in,
                                        const CVParamTransfer& param,
                                        CObjectReference* selfRef);

}

