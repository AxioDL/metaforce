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
    struct NameEntry : public BigDNA
    {
        DECL_DNA
        HECL::FourCC type;
        UniqueID32 id;
        Value<atUint32> nameLen;
        String<DNA_COUNT(nameLen)> name;
    };

    struct Entry : public BigDNA
    {
        DECL_DNA
        Value<atUint32> compressed;
        HECL::FourCC type;
        UniqueID32 id;
        Value<atUint32> size;
        Value<atUint32> offset;
    };

private:
    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::unordered_map<UniqueID32, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;
    Delete expl;

public:
    void read(Athena::io::IStreamReader& reader)
    {
        reader.setEndian(Athena::BigEndian);
        atUint32 version = reader.readUint32();
        if (version != 0x00030005)
            LogModule.report(LogVisor::FatalError, "unexpected PAK magic");
        reader.readUint32();

        atUint32 nameCount = reader.readUint32();
        m_nameEntries.clear();
        m_nameEntries.reserve(nameCount);
        for (atUint32 n=0 ; n<nameCount ; ++n)
        {
            m_nameEntries.emplace_back();
            m_nameEntries.back().read(reader);
        }

        atUint32 count = reader.readUint32();
        m_entries.clear();
        m_entries.reserve(count);
        m_idMap.clear();
        m_idMap.reserve(count);
        for (atUint32 e=0 ; e<count ; ++e)
        {
            m_entries.emplace_back();
            m_entries.back().read(reader);
            m_idMap[m_entries.back().id] = &m_entries.back();
        }

        m_nameMap.clear();
        m_nameMap.reserve(nameCount);
        for (NameEntry& entry : m_nameEntries)
        {
            std::unordered_map<UniqueID32, Entry*>::iterator found = m_idMap.find(entry.id);
            if (found != m_idMap.end())
                m_nameMap[entry.name] = found->second;
        }
    }
    void write(Athena::io::IStreamWriter& writer)
    {
        writer.setEndian(Athena::BigEndian);
        writer.writeUint32(0x00030005);
        writer.writeUint32(0);

        writer.writeUint32(m_nameEntries.size());
        for (NameEntry& entry : m_nameEntries)
        {
            entry.nameLen = entry.name.size();
            entry.write(writer);
        }

        writer.writeUint32(m_entries.size());
        for (const Entry& entry : m_entries)
            entry.write(writer);
    }

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
