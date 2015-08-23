#ifndef __DNAMP3_PAK_HPP__
#define __DNAMP3_PAK_HPP__

#include <unordered_map>

#include <lzo/lzo1x.h>
#include <NOD/DiscBase.hpp>
#include "../DNACommon/PAK.hpp"

namespace Retro
{
namespace DNAMP3
{

extern const HECL::FourCC CMPD;

struct PAK : BigDNA
{
    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> version;
        Value<atUint32> headSz;
        Value<atUint8> md5sum[16];
        Seek<40, Athena::Current> seek;
    } m_header;

    struct NameEntry : BigDNA
    {
        DECL_DNA
        String<-1> name;
        DNAFourCC type;
        UniqueID64 id;
    };

    struct Entry : BigDNA
    {
        DECL_DNA
        Value<atUint32> compressed;
        DNAFourCC type;
        UniqueID64 id;
        Value<atUint32> size;
        Value<atUint32> offset;
        UniqueResult unique;

        std::unique_ptr<atUint8[]> getBuffer(const NOD::DiscBase::IPartition::Node& pak, atUint64& szOut) const;
        inline PAKEntryReadStream beginReadStream(const NOD::DiscBase::IPartition::Node& pak, atUint64 off=0) const
        {
            atUint64 sz;
            std::unique_ptr<atUint8[]> buf = getBuffer(pak, sz);
            return PAKEntryReadStream(std::move(buf), sz, off);
        }
    };

    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::unordered_map<UniqueID64, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;

    DECL_EXPLICIT_DNA

    inline const Entry* lookupEntry(const UniqueID64& id) const
    {
        std::unordered_map<UniqueID64, Entry*>::const_iterator result = m_idMap.find(id);
        if (result != m_idMap.end())
            return result->second;
        return nullptr;
    }

    inline const Entry* lookupEntry(const std::string& name) const
    {
        std::unordered_map<std::string, Entry*>::const_iterator result = m_nameMap.find(name);
        if (result != m_nameMap.end())
            return result->second;
        return nullptr;
    }

    inline std::string bestEntryName(const Entry& entry) const
    {
        /* Prefer named entries first */
        for (const NameEntry& nentry : m_nameEntries)
            if (nentry.id == entry.id)
                return nentry.name;

        /* Otherwise return ID format string */
        return entry.type.toString() + '_' + entry.id.toString();
    }

    typedef UniqueID64 IDType;
};

}
}

#endif // __DNAMP3_PAK_HPP__
