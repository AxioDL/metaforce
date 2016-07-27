#ifndef __URDE_CPVSVISSET_HPP__
#define __URDE_CPVSVISSET_HPP__

#include "RetroTypes.hpp"
#include <memory>

namespace urde
{

class CPVSVisSet
{
    int x0_bitCount;
    int x4_setCount = 0;
    std::vector<unsigned char> x8_bitset;
public:
    CPVSVisSet(int count) : x0_bitCount(count) {}
    CPVSVisSet(int a, std::vector<unsigned char>&& bitset)
    : x0_bitCount(1), x4_setCount(a), x8_bitset(std::move(bitset)) {}
    void Reset(bool);
    bool GetVisible(int);
    void SetVisible(int,bool);
    void SetFromMemory(const unsigned char*);
};

}

#endif // __URDE_CPVSVISSET_HPP__
