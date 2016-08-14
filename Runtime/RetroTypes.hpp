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
using ResId = s64;

struct SObjectTag
{
    FourCC type;
    ResId id = -1;
    operator bool() const {return id != -1;}
    bool operator!=(const SObjectTag& other) const {return id != other.id;}
    bool operator==(const SObjectTag& other) const {return id == other.id;}
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

using TUniqueId = s16;
using TAreaId = s32;

struct TEditorId
{
    TEditorId() = default;
    TEditorId(u32 idin) : id(idin) {}
    u32 id = -1;
    u8 LayerNum() const { return (id >> 26) & 0x3f; }
    u16 AreaNum() const { return (id >> 16) & 0x3ff; }
    TUniqueId Id() const { return id & 0xffff; }

    bool operator<(const TEditorId& other) const { return (id & 0x3ffffff) < (other.id & 0x3ffffff); }
    bool operator!=(const TEditorId& other) const { return (id & 0x3ffffff) != (other.id & 0x3ffffff); }
    bool operator==(const TEditorId& other) const { return (id & 0x3ffffff) == (other.id & 0x3ffffff); }
};

#define kInvalidEditorId TEditorId()
#define kInvalidUniqueId TUniqueId(-1)
#define kInvalidAreaId TAreaId(-1)

}

namespace std
{
template<>
struct hash<urde::SObjectTag>
{
    inline size_t operator()(const urde::SObjectTag& tag) const
    {return tag.id;}
};
}

#endif // __URDE_TYPES_HPP__
