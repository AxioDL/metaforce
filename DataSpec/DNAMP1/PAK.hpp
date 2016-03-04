#ifndef __DNAMP1_PAK_HPP__
#define __DNAMP1_PAK_HPP__

#include <unordered_map>

#include <nod/DiscBase.hpp>
#include "../DNACommon/PAK.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct PAK : BigDNA
{
    bool m_useLzo;
    bool m_noShare;
    PAK(bool useLzo, bool noShare) : m_useLzo(useLzo), m_noShare(noShare) {}
    DECL_EXPLICIT_DNA

    struct NameEntry : BigDNA
    {
        DECL_DNA
        DNAFourCC type;
        UniqueID32 id;
        Value<atUint32> nameLen;
        String<DNA_COUNT(nameLen)> name;
    };

    struct Entry : BigDNA
    {
        DECL_DNA
        Value<atUint32> compressed;
        DNAFourCC type;
        UniqueID32 id;
        Value<atUint32> size;
        Value<atUint32> offset;
        UniqueResult unique;
        std::string name; /* backreferencing name for RE purposes */

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
    std::unordered_map<UniqueID32, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;

    const Entry* lookupEntry(const UniqueID32& id) const
    {
        std::unordered_map<UniqueID32, Entry*>::const_iterator result = m_idMap.find(id);
        if (result != m_idMap.end())
            return result->second;
        return nullptr;
    }

    const Entry* lookupEntry(const std::string& name) const
    {
        std::unordered_map<std::string, Entry*>::const_iterator result = m_nameMap.find(name);
        if (result != m_nameMap.end())
            return result->second;
        return nullptr;
    }

    std::string bestEntryName(const Entry& entry) const
    {
        /* Prefer named entries first */
        for (const NameEntry& nentry : m_nameEntries)
            if (nentry.id == entry.id)
                return nentry.name;

        /* Otherwise return ID format string */
        return entry.type.toString() + '_' + entry.id.toString();
    }

    using IDType = UniqueID32;
};

}
}

#endif // __DNAMP1_PAK_HPP__
