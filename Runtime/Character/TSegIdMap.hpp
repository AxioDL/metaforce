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
    CSegId x1_curPrevBone = 0;
    u32 x4_capacity = 100;
    CSegId x8_prevBones[100];
    T x6c_bones[100];
public:
    T& operator[](const CSegId& id) {return SetElement(id);}
    const T& operator[](const CSegId& id) const {return x6c_bones[id];}
    T& SetElement(const CSegId& id, T&& obj)
    {
        x6c_bones[id] = std::move(obj);
        if (x8_prevBones[id] == 0xff)
        {
            x8_prevBones[id] = x1_curPrevBone;
            x1_curPrevBone = id;
            ++x0_boneCount;
        }
        return x6c_bones[id];
    }
    T& SetElement(const CSegId& id)
    {
        if (x8_prevBones[id] == 0xff)
        {
            x8_prevBones[id] = x1_curPrevBone;
            x1_curPrevBone = id;
            ++x0_boneCount;
        }
        return x6c_bones[id];
    }
    void DelElement(const CSegId& id)
    {
        if (x8_prevBones[id] != 0xff)
        {
            if (id == x1_curPrevBone)
                x1_curPrevBone = x8_prevBones[id];
            x8_prevBones[id] = 0xff;
            --x0_boneCount;
        }
    }
    bool HasElement(const CSegId& id) const {return x8_prevBones[id] != 0xff;}
};

}

#endif // __URDE_TSEGIDMAP_HPP__
