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
    enum class EEntranceType : u32
    {
        FacePlayer,
        UseOrientation
    };
private:
public:
    CBeetle(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, CPatterned::EFlavorType,EEntranceType, const CDamageInfo &, const CDamageVulnerability&,
            const zeus::CVector3f&, float, float, float, const CDamageVulnerability&, const CActorParameters&,
            const std::experimental::optional<CStaticRes>);

    void Accept(IVisitor &visitor);
};
}
}

#endif // CBEETLE_HPP
