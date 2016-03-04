#ifndef __DNAMP3_PAK_HPP__
#define __DNAMP3_PAK_HPP__

#include <unordered_map>

#include <lzo/lzo1x.h>
#include <nod/DiscBase.hpp>
#include "../DNACommon/PAK.hpp"

namespace DataSpec
{
namespace DNAMP3
{

extern const hecl::FourCC CMPD;

struct PAK : BigDNA
{
    bool m_noShare;
    PAK(bool noShare) : m_noShare(noShare) {}

    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> version;
        Value<atUint32> headSz;
        Value<atUint8> md5sum[16];
        Seek<40, athena::Current> seek;
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
        std::string name;

        std::unique_ptr<atUint8[]> getBuffer(const nod::Node& pak, atUint64& szOut) const;
        inline PAKEntryReadStream beginReadStream(const nod::Node& pak, atUint64 off=0) const
        {
            atUint64 sz;
            std::unique_ptr<atUint8[]> buf = getBuffer(pak, sz);
            return PAKEntryReadStream(std::move(buf), sz, off);
        }
    };

    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::vector<Entry*> m_firstEntries;
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
