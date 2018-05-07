#ifndef __URDE_MP1_CPARASITE_HPP__
#define __URDE_MP1_CPARASITE_HPP__

#include "World/CWallWalker.hpp"

namespace urde
{
class CModelData;
}

namespace urde::MP1
{
struct CParasiteInfo
{
    u32 x0_ = 0;
    union
    {
        struct
        {
            bool x4_24_ : 1; bool x4_26_ : 1;
        };
        u32 x4_dummy = 0;
    };
    float x8_ = 0.f;
    float xc_ = 0.f;
    float x10_ = 0.f;
    float x14_ = 0.f;
    union
    {
        struct
        {
            bool x18_24_ : 1;
        };
        u32 x18_dummy = 0;
    };
};

class CParasite : public CWallWalker
{
public:
    CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info, const zeus::CTransform& xf,
              CModelData&& mData, const CPatternedInfo&, u32, float, float, float, float, float, float, float, float, float,
              float, float, float, float, float, float, float, float, float, bool, u32, const CDamageVulnerability&, const CParasiteInfo&, u16, u16,
              u16, u32, u32, float, const CActorParameters&);

    void Accept(IVisitor&);
};
}
#endif // __URDE_MP1_CPARASITE_HPP__
