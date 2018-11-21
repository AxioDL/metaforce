#pragma once

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
    SoundInt32 = 4,
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
    s32 x24_index;
    bool x28_unique;
    float x2c_weight;
    s32 x30_charIdx = -1;
    s32 x34_flags;
public:
    CPOINode(std::string_view name, EPOIType type, const CCharAnimTime& time,
             s32 index, bool unique, float weight, s32 charIdx, s32 flags);
    CPOINode(CInputStream& in);
    virtual ~CPOINode() = default;

    std::string_view GetString() const { return x8_name; }
    const CCharAnimTime& GetTime() const { return x1c_time; }
    void SetTime(const CCharAnimTime& time) { x1c_time = time; }
    EPOIType GetPoiType() const { return x18_type; }
    s32 GetIndex() const { return x24_index; }
    bool GetUnique() const { return x28_unique; }
    float GetWeight() const { return x2c_weight; }
    s32 GetCharacterIndex() const { return x30_charIdx; }
    s32 GetFlags() const { return x34_flags; }

    bool operator>(const CPOINode& other) const;
    bool operator<(const CPOINode& other) const;
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

