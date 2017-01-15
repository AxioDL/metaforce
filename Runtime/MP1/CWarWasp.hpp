#ifndef CWARWASP_HPP
#define CWARWASP_HPP

#include "World/CPatterned.hpp"

namespace urde
{
class CDamageInfo;
namespace MP1
{
class CWarWasp : public CPatterned
{
public:
    CWarWasp(TUniqueId uid, const std::string& name, const CEntityInfo& info,
             const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
             CPatterned::EFlavorType flavor, CPatterned::EColliderType, const CDamageInfo& dInfo1, const CActorParameters&,
             ResId weapon, const CDamageInfo& dInfo2, ResId particle, u32 w3);

    void Accept(IVisitor& visitor);
};
}
}

#endif // CWARWASP_HPP
