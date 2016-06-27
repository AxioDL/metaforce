#ifndef __URDE_CCOLLISIONINFOLIST_HPP__
#define __URDE_CCOLLISIONINFOLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CCollisionInfo;
class CCollisionInfoList
{
    rstl::reserved_vector<CCollisionInfo, 32> x0_list;
public:
    CCollisionInfoList() = default;

    void GetAverageLeftNormal() const;
    void GetAveragePoint() const;
    void GetUnionOfAllLeftMaterials() const;
    s32 GetCount() const;
    void Swap(s32);

    void Add(const CCollisionInfo&, bool);
    void Clear();
    void End();
    void End() const;
    void Begin();
    void Begin() const;
};
}

#endif // __URDE_CCOLLISIONINFOLIST_HPP__
