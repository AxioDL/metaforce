#ifndef __URDE_TSEGIDMAP_HPP__
#define __URDE_TSEGIDMAP_HPP__

#include <map>
#include "CSegId.hpp"

namespace urde
{

template <class T>
class TSegIdMap
{
    CSegId x0_boneCount = 0;
    CSegId x1_capacity = 0;
    u32 x4_maxCapacity = 100;
    std::pair<CSegId, CSegId> x8_indirectionMap[100];
    std::unique_ptr<T[]> xd0_bones;
    CSegId xd4_curPrevBone = 0;
public:
    TSegIdMap(const CSegId& capacity) : x1_capacity(capacity), xd0_bones(new T[capacity]) {}
    T& operator[](const CSegId& id) {return SetElement(id);}
    const T& operator[](const CSegId& id) const {return xd0_bones[id];}
    T& SetElement(const CSegId& id, T&& obj)
    {
        xd0_bones[id] = std::move(obj);
        if (x8_indirectionMap[id].first == 0xff)
        {
            x8_indirectionMap[id].first = xd4_curPrevBone;
            x8_indirectionMap[id].second = x0_boneCount;
            xd4_curPrevBone = id;
            ++x0_boneCount;
        }
        return xd0_bones[id];
    }
    T& SetElement(const CSegId& id)
    {
        if (x8_indirectionMap[id].first == 0xff)
        {
            x8_indirectionMap[id].first = xd4_curPrevBone;
            x8_indirectionMap[id].second = x0_boneCount;
            xd4_curPrevBone = id;
            ++x0_boneCount;
        }
        return xd0_bones[id];
    }
    void DelElement(const CSegId& id)
    {
        if (x8_indirectionMap[id].first != 0xff)
        {
            if (id == xd4_curPrevBone)
                xd4_curPrevBone = x8_indirectionMap[id].first;
            x8_indirectionMap[id].first = 0xff;
            x8_indirectionMap[id].second = 0xff;
            --x0_boneCount;
        }
    }
    bool HasElement(const CSegId& id) const {return x8_indirectionMap[id].first != 0xff;}

    u32 GetCapacity() const { return x1_capacity; }
};

}

#endif // __URDE_TSEGIDMAP_HPP__
