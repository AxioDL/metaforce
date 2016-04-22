#ifndef __URDE_CMATERIALLIST_HPP__
#define __URDE_CMATERIALLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{
enum class EMaterialTypes
{
    Six = 6,
    Eight = 8,
    Ten = 10,
    Eleven = 11,
    Twelve = 12,
    Thirten = 13,
    Fourteen = 14,
    Fifteen = 15,
    Nineteen = 19,
    ThirtyTwo = 32,
    ThirtyFour = 34,
    FourtyOne = 41,
    FourtyTwo = 42,
    FourtyThree = 43,
    FourtyEight = 48,
    FourtyNine = 49,
    SixtyThree = 63
};

class CMaterialList
{
    friend class CMaterialFilter;
    u64 x0_ = 0;
public:
    CMaterialList() = default;
    CMaterialList(u64 flags) : x0_(flags) {}
    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5, EMaterialTypes t6)
        : CMaterialList(t1, t2, t3, t4, t5)
    { x0_ = 1ull << u64(t6); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5)
        : CMaterialList(t1, t2, t3, t4)
    { x0_ = 1ull << u64(t5); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4)
        : CMaterialList(t1, t2, t3)
    { x0_ = 1ull << u64(t4); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3)
        : CMaterialList(t1, t2)
    { x0_ = 1ull << u64(t3); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2)
        : CMaterialList(t1)
    { x0_ = 1ull << u64(t2); }

    CMaterialList(EMaterialTypes t1)
        : x0_(1ull << u64(t1))
    {
    }

    static u32 BitPosition(u64 flag)
    {
        for (u32 i = 0; i < 63; ++i)
            if ((flag & (1ull << i)) != 0)
                return i;
        return -1;
    }

    void Add(EMaterialTypes type)
    {
        x0_ |= (1ull << u64(type));
    }

    void Remove(EMaterialTypes type)
    {
        x0_ &= ~(1ull << u64(type));
    }

    void Remove(const CMaterialList& other)
    {
        x0_ &= ~(other.x0_);
    }

    bool HasMaterial(EMaterialTypes type)
    {
        return (x0_ & (1ull << u64(type))) != 0;
    }

    bool SharesMaterials(const CMaterialList& other)
    {
        for (u32 i = 0; i < 64; i++)
        {
            if ((x0_ & (1ull << i)) != 0 && (other.x0_ & (1ull << i)) != 0)
                return true;
        }

        return false;
    }
};

}

#endif // __URDE_CMATERIALLIST_HPP__
