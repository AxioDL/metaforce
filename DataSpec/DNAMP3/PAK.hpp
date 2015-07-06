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
    struct Header : public BigDNA
    {
        Value<atUint32> version;
        Value<atUint32> headSz;
        Value<atUint8> md5sum[16];
        Seek<40, Athena::Current> seek;
    } m_header;

    struct NameEntry : public BigDNA
    {
        String<-1> name;
        HECL::FourCC type;
        UniqueID64 id;
    };

    struct Entry : public BigDNA
    {
        atUint32 compressed;
        HECL::FourCC type;
        UniqueID64 id;
        atUint32 size;
        atUint32 offset;
    };

private:
    std::vector<NameEntry> m_nameEntries;
    std::vector<Entry> m_entries;
    std::unordered_map<UniqueID64, Entry*> m_idMap;
    std::unordered_map<std::string, Entry*> m_nameMap;
    size_t m_dataOffset = 0;
    Delete expl;

public:
    void read(Athena::io::IStreamReader& reader)
    {
        reader.setEndian(Athena::BigEndian);
        m_header.read(reader);
        if (m_header.version != 2)
            LogModule.report(LogVisor::FatalError, "unexpected PAK magic");

        reader.seek(8, Athena::Current);
        atUint32 strgSz = reader.readUint32();
        reader.seek(4, Athena::Current);
        atUint32 rshdSz = reader.readUint32();
        reader.seek(44, Athena::Current);
        m_dataOffset = 128 + strgSz + rshdSz;

        atUint32 nameCount = reader.readUint32();
        m_nameEntries.clear();
        m_nameEntries.reserve(nameCount);
        for (atUint32 n=0 ; n<nameCount ; ++n)
        {
            m_nameEntries.emplace_back();
            m_nameEntries.back().read(reader);
        }
        reader.seek((reader.position() + 63) & ~63, Athena::Begin);

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
            Entry* found = m_idMap.find(entry.id);
            if (found != m_idMap.end())
                m_nameMap[entry.name] = found;
        }
    }
    void write(Athena::io::IStreamWriter& writer)
    {
        writer.setEndian(Athena::BigEndian);
        m_header.write(writer);

        HECL::FourCC("STRG").write(writer);
        atUint32 strgSz = 4;
        for (NameEntry& entry : m_nameEntries)
            strgSz += entry.name.size() + 13;
        atUint32 strgPad = ((strgSz + 63) & ~63) - strgSz;
        strgSz += strgPad;
        writer.writeUint32(strgSz);

        HECL::FourCC("RSHD").write(writer);
        atUint32 rshdSz = 4 + 24 * m_entries.size();
        atUint32 rshdPad = ((rshdSz + 63) & ~63) - rshdSz;
        rshdSz += rshdPad;
        writer.writeUint32(rshdSz);

        HECL::FourCC("DATA").write(writer);
        atUint32 dataSz = 0;
        for (Entry& entry : m_entries)
            dataSz += (entry.size + 63) & ~63;
        atUint32 dataPad = ((dataSz + 63) & ~63) - dataSz;
        dataSz += dataPad;
        writer.writeUint32(dataSz);
        writer.seek(36, Athena::Current);

        writer.writeUint32(m_nameEntries.size());
        for (NameEntry& entry : m_nameEntries)
            entry.write(writer);
        writer.seek(strgPad, Athena::Current);

        writer.writeUint32(m_entries.size());
        for (Entry& entry : m_entries)
            entry.write(writer);
        writer.seek(rshdPad, Athena::Current);
    }

    inline const Entry* lookupEntry(const UniqueID64& id) const
    {
        Entry* result = m_idMap.find(id);
        if (result != m_idMap.end())
            return result;
        return nullptr;
    }

    inline const Entry* lookupEntry(const std::string& name) const
    {
        Entry* result = m_nameMap.find(name);
        if (result != m_nameMap.end())
            return result;
        return nullptr;
    }

    inline size_t getDataOffset() const {return m_dataOffset;}
};

}
}

#endif // __DNAMP3_PAK_HPP__
