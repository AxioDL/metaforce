#ifndef __DNAMP1_PAK_HPP__
#define __DNAMP1_PAK_HPP__

#include <unordered_map>

#include "../Logging.hpp"
#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

class PAK : public BigDNA
{
public:
    DECL_EXPLICIT_DNA

    struct NameEntry : public BigDNA
    {
        DECL_DNA
        FourCC type;
        UniqueID32 id;
        Value<atUint32> nameLen;
        String<DNA_COUNT(nameLen)> name;
    };

    struct Entry : BigDNA
    {
        DECL_DNA
        Value<atUint32> compressed;
        FourCC type;
        UniqueID32 id;
        Value<atUint32> size;
        Value<atUint32> offset;
    };

    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::unordered_map<UniqueID32, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;

    inline const Entry* lookupEntry(const UniqueID32& id) const
    {
        std::unordered_map<UniqueID32, Entry*>::const_iterator result = m_idMap.find(id);
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
};

}
}

#endif // __DNAMP1_PAK_HPP__
