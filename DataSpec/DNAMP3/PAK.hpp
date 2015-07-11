#ifndef __DNAMP3_PAK_HPP__
#define __DNAMP3_PAK_HPP__

#include <unordered_map>

#include "../Logging.hpp"
#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP3
{

class PAK : public BigDNA
{
public:
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
        FourCC type;
        UniqueID64 id;
    };

    struct Entry : BigDNA
    {
        DECL_DNA
        Value<atUint32> compressed;
        HECL::FourCC type;
        UniqueID64 id;
        Value<atUint32> size;
        Value<atUint32> offset;
    };

private:
    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::unordered_map<UniqueID64, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;
    size_t m_dataOffset = 0;

public:
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

    inline std::vector<Entry>::iterator begin() {return m_entries.begin();}
    inline std::vector<Entry>::iterator end() {return m_entries.end();}
    inline size_t getDataOffset() const {return m_dataOffset;}
};

}
}

#endif // __DNAMP3_PAK_HPP__
