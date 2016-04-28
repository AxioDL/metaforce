#ifndef __URDE_CPOINODE_HPP__
#define __URDE_CPOINODE_HPP__

#include "IOStreams.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

class CPOINode
{
protected:
    u16 x4_ = 1;
    std::string x8_name;
    u16 x18_;
    CCharAnimTime x1c_time;
    u32 x24_index;
    bool x28_;
    float x2c_weight;
    u32 x30_;
    u32 x34_;
public:
    CPOINode(const std::string& name, u16, const CCharAnimTime& time, u32 index, bool, float weight, u32, u32);
    CPOINode(CInputStream& in);
    virtual ~CPOINode() = default;

    const std::string& GetName() const {return x8_name;}
    const CCharAnimTime& GetTime() const {return x1c_time;}
    u32 GetIndex() const {return x24_index;}
    float GetWeight() const { return x2c_weight; }
};

}

#endif // __URDE_CPOINODE_HPP__
