#ifndef __URDE_CCOLLISIONINFOLIST_HPP__
#define __URDE_CCOLLISIONINFOLIST_HPP__

#include "RetroTypes.hpp"
#include "CCollisionInfo.hpp"

namespace urde
{
class CCollisionInfoList
{
    rstl::reserved_vector<CCollisionInfo, 32> x0_list;
public:
    CCollisionInfoList() = default;

    void GetAverageLeftNormal() const;
    void GetAveragePoint() const;
    void GetUnionOfAllLeftMaterials() const;
    s32 GetCount() const { return x0_list.size(); }
    void Swap(s32);

    void Add(const CCollisionInfo& info, bool swap)
    {
        if (x0_list.size() == 32)
            return;
        if (!swap)
            x0_list.push_back(info);
        else
            x0_list.push_back(info.GetSwapped());
    }
    void Clear() { x0_list.clear(); }
    const CCollisionInfo& Front() const { return x0_list.front(); }
    const CCollisionInfo& GetItem(int i) const { return x0_list[i]; }
    rstl::reserved_vector<CCollisionInfo, 32>::iterator end() { return x0_list.end(); }
    rstl::reserved_vector<CCollisionInfo, 32>::const_iterator end() const { return x0_list.end(); }
    rstl::reserved_vector<CCollisionInfo, 32>::iterator begin() { return x0_list.begin(); }
    rstl::reserved_vector<CCollisionInfo, 32>::const_iterator begin() const { return x0_list.begin(); }
};
}

#endif // __URDE_CCOLLISIONINFOLIST_HPP__
