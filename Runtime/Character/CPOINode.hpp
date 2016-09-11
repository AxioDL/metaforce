#ifndef __URDE_CPOINODE_HPP__
#define __URDE_CPOINODE_HPP__

#include "IOStreams.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{
class IAnimSourceInfo;

enum class EPOIType : u16
{
    Loop = 0,
    EmptyBool = 1,
    EmptyInt32 = 2,
    Particle = 5,
    UserEvent = 6,
    RandRate = 7,
    Sound = 8,
};

class CPOINode
{
protected:
    u16 x4_ = 1;
    std::string x8_name;
    EPOIType x18_type;
    CCharAnimTime x1c_time;
    u32 x24_index;
    bool x28_;
    float x2c_weight;
    u32 x30_charIdx = -1;
    u32 x34_flags;
public:
    CPOINode(const std::string& name, EPOIType type, const CCharAnimTime& time,
             u32 index, bool, float weight, u32 charIdx, u32 flags);
    CPOINode(CInputStream& in);
    virtual ~CPOINode() = default;

    const std::string& GetString() const {return x8_name;}
    const CCharAnimTime& GetTime() const {return x1c_time;}
    void SetTime(const CCharAnimTime& time) { x1c_time = time; }
    EPOIType GetPoiType() const { return x18_type; }
    u32 GetIndex() const {return x24_index;}
    float GetWeight() const { return x2c_weight; }
    u32 GetCharacterIndex() const { return x30_charIdx; }
    u32 GetFlags() const { return x34_flags; }

    bool operator>(const CPOINode& other) const;
    bool operator<(const CPOINode& other) const;
    static bool compare(const CPOINode& a, const CPOINode& b);
};

template <class T>
u32 _getPOIList(const CCharAnimTime& time,
                T* listOut,
                u32 capacity, u32 iterator, u32 unk1,
                const std::vector<T>& stream,
                const CCharAnimTime& curTime,
                const IAnimSourceInfo& animInfo, u32 passedCount);

template <class T>
u32 _getPOIList(const CCharAnimTime& time,
                T* listOut,
                u32 capacity, u32 iterator, u32 unk1,
                const std::vector<T>& stream,
                const CCharAnimTime& curTime);

}

#endif // __URDE_CPOINODE_HPP__
