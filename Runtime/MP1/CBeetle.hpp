#ifndef CBEETLE_HPP
#define CBEETLE_HPP

#include "World/CPatterned.hpp"

namespace urde
{
class CDamageInfo;

namespace MP1
{
class CBeetle : public CPatterned
{
public:
    enum EEntranceType
    {
    };
private:
public:
    CBeetle(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, CPatterned::EFlavorType,EEntranceType, const CDamageInfo &, const CDamageVulnerability&,
            const zeus::CVector3f&, float, float, float, const CDamageVulnerability&, const CActorParameters&,
            const rstl::optional_object<CStaticRes>);
};
}
}

#endif // CBEETLE_HPP
