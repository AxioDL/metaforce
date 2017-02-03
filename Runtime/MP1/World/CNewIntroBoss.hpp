#ifndef __URDE_MP1_CNEWINTROBOSS_HPP__
#define __URDE_MP1_CNEWINTROBOSS_HPP__

#include "World/CPatterned.hpp"

namespace urde
{
class CDamageInfo;

namespace MP1
{

class CNewIntroBoss : public CPatterned
{
public:
    CNewIntroBoss(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                  const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                  const CActorParameters& actParms, float, u32, const CDamageInfo& dInfo,
                  u32, u32, u32, u32);

    void Accept(IVisitor &visitor);
};

}
}

#endif // __URDE_MP1_CNEWINTROBOSS_HPP__
