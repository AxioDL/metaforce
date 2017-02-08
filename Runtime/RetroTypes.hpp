#ifndef __URDE_TYPES_HPP__
#define __URDE_TYPES_HPP__

#include <vector>
#include <utility>
#include <string>
#include "GCNTypes.hpp"
#include "rstl.hpp"
#include "DataSpec/DNACommon/DNACommon.hpp"
#include "IOStreams.hpp"

namespace urde
{

using FourCC = hecl::FourCC;
using ResId = u64;

struct SObjectTag
{
    FourCC type;
    ResId id = -1;
    operator bool() const { return (id & 0xffffffff) != 0xffffffff; }
    bool operator!=(const SObjectTag& other) const { return id != other.id; }
    bool operator==(const SObjectTag& other) const { return id == other.id; }
    bool operator<(const SObjectTag& other) const { return id < other.id; }
    SObjectTag() = default;
    SObjectTag(FourCC tp, ResId rid) : type(tp), id(rid) {}
    SObjectTag(CInputStream& in)
    {
        in.readBytesToBuf(&type, 4);
        id = in.readUint32Big();
    }
    void readMLVL(CInputStream& in)
    {
        id = in.readUint32Big();
        in.readBytesToBuf(&type, 4);
    }
};

struct TEditorId
{
    TEditorId() = default;
    TEditorId(u32 idin) : id(idin) {}
    u32 id = -1;
    u8 LayerNum() const { return (id >> 26) & 0x3f; }
    u16 AreaNum() const { return (id >> 16) & 0x3ff; }
    u16 Id() const { return id & 0xffff; }

    bool operator<(const TEditorId& other) const { return (id & 0x3ffffff) < (other.id & 0x3ffffff); }
    bool operator!=(const TEditorId& other) const { return (id & 0x3ffffff) != (other.id & 0x3ffffff); }
    bool operator==(const TEditorId& other) const { return (id & 0x3ffffff) == (other.id & 0x3ffffff); }
};

using TUniqueId = s16;
using TAreaId = s32;

#define kInvalidEditorId TEditorId()
#define kInvalidUniqueId TUniqueId(-1)
#define kInvalidAreaId TAreaId(-1)
}

#if 0
template <class T, size_t N>
class TRoundRobin
{
    rstl::reserved_vector<T, N> vals;

public:
    TRoundRobin(const T& val) : vals(N, val) {}

    void PushBack(const T& val) { vals.push_back(val); }

    size_t Size() const { return vals.size(); }

    const T& GetLastValue() const { return vals.back(); }

    void Clear() { vals.clear(); }

    const T& GetValue(s32) const {}
};
#endif

template <class T>
T GetAverage(const T* v, s32 count)
{
    T r = v[0];
    for (s32 i = 1; i < count; ++i)
        r += v[i];

    return r / count;
}

template <class T, size_t N>
class TReservedAverage : rstl::reserved_vector<T, N>
{
public:
    TReservedAverage() = default;
    TReservedAverage(const T& t) { resize(N, t); }

    void AddValue(const T& t)
    {
        if (this->size() < N)
            this->push_back(t);
    }

    rstl::optional_object<T> GetAverage() const
    {
        if (this->empty())
            return {};

        return {::GetAverage<T>(this->data(), this->size())};
    }

    void Clear() { this->clear(); }
};

namespace std
{
template <>
struct hash<urde::SObjectTag>
{
    inline size_t operator()(const urde::SObjectTag& tag) const { return tag.id; }
};
}

#endif // __URDE_TYPES_HPP__
