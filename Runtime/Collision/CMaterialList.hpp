#ifndef __URDE_CMATERIALLIST_HPP__
#define __URDE_CMATERIALLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{
enum class EMaterialTypes
{
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Nine = 9,
    Ten = 10,
    Eleven = 11,
    Twelve = 12,
    Thirten = 13,
    Fourteen = 14,
    Fifteen = 15,
    Nineteen = 19,
    TwentyOne = 21,
    ThirtyTwo = 32,
    ThirtyThree = 33,
    ThirtyFour = 34,
    ThirtySeven = 37,
    ThirtyEight = 38,
    ThirtyNine = 39,
    Fourty = 40,
    FourtyOne = 41,
    FourtyTwo = 42,
    FourtyThree = 43,
    FourtyFour = 44,
    FourtyEight = 48,
    FourtyNine = 49,
    Fifty = 50,
    FiftySix = 56,
    SixtyThree = 63
};

class CMaterialList
{
    friend class CMaterialFilter;
    u64 x0_list = 0;
public:
    CMaterialList() = default;
    CMaterialList(u64 flags) : x0_list(flags) {}
    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5, EMaterialTypes t6)
        : CMaterialList(t1, t2, t3, t4, t5)
    { x0_list = 1ull << u64(t6); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5)
        : CMaterialList(t1, t2, t3, t4)
    { x0_list = 1ull << u64(t5); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4)
        : CMaterialList(t1, t2, t3)
    { x0_list = 1ull << u64(t4); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3)
        : CMaterialList(t1, t2)
    { x0_list = 1ull << u64(t3); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2)
        : CMaterialList(t1)
    { x0_list = 1ull << u64(t2); }

    CMaterialList(EMaterialTypes t1)
        : x0_list(1ull << u64(t1))
    {
    }

    static u32 BitPosition(u64 flag)
    {
        u32 high = *((u32*)(&flag)[0]);
        u32 low = *((u32*)(&flag)[1]);
        for (u32 i = 0; i < 8; ++i)
        {
        }
        return -1;
    }

    void Add(EMaterialTypes type)
    {
        x0_list |= (1ull << u64(type));
    }

    void Remove(EMaterialTypes type)
    {
        x0_list &= ~(1ull << u64(type));
    }

    void Remove(const CMaterialList& other)
    {
        x0_list &= ~(other.x0_list);
    }

    bool HasMaterial(EMaterialTypes type)
    {
        return (x0_list & (1ull << u64(type))) != 0;
    }

    bool SharesMaterials(const CMaterialList& other)
    {
        for (u32 i = 0; i < 64; i++)
        {
            if ((x0_list & (1ull << i)) != 0 && (other.x0_list & (1ull << i)) != 0)
                return true;
        }

        return false;
    }
};

}

#endif // __URDE_CMATERIALLIST_HPP__
