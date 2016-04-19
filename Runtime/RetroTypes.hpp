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
using TEditorId = s32;
using TAreaId = s32;
using TGameScriptId = s32;

#define kInvalidEditorId TEditorId(-1)
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
