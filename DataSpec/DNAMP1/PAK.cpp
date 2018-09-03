#include <zlib.h>
#include <lzo/lzo1x.h>
#include "DNAMP1.hpp"
#include "PAK.hpp"
#include "AGSC.hpp"

namespace DataSpec::DNAMP1
{

template <>
void PAK::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{
    atUint32 version = reader.readUint32Big();
    if (version != 0x00030005)
        Log.report(logvisor::Fatal, "unexpected PAK magic");
    reader.readUint32Big();

    atUint32 nameCount = reader.readUint32Big();
    m_nameEntries.clear();
    m_nameEntries.reserve(nameCount);
    for (atUint32 n=0 ; n<nameCount ; ++n)
    {
        m_nameEntries.emplace_back();
        m_nameEntries.back().read(reader);
    }

    atUint32 count = reader.readUint32Big();
    m_entries.clear();
    m_entries.reserve(count);
    m_firstEntries.clear();
    m_firstEntries.reserve(count);
    std::vector<Entry> entries;
    entries.reserve(count);
    for (atUint32 e=0 ; e<count ; ++e)
    {
        entries.emplace_back();
        entries.back().read(reader);
    }
    for (atUint32 e=0 ; e<count ; ++e)
    {
        Entry& entry = entries[e];
        if (entry.compressed && m_useLzo)
            entry.compressed = 2;

        auto search = m_entries.find(entry.id);
        if (search == m_entries.end())
        {
            m_firstEntries.push_back(entry.id);
            m_entries[entry.id] = std::move(entry);
        }
        else
        {
            /* Find next MREA to record which area has dupes */
            for (atUint32 e2=e+1 ; e2<count ; ++e2)
            {
                Entry& entry2 = entries[e2];
                if (entry2.type != FOURCC('MREA'))
                    continue;
                m_dupeMREAs.insert(entry2.id);
                break;
            }
        }
    }

    m_nameMap.clear();
    m_nameMap.reserve(nameCount);
    for (NameEntry& entry : m_nameEntries)
        m_nameMap[entry.name] = entry.id;
}

template <>
void PAK::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{
    writer.writeUint32Big(0x00030005);
    writer.writeUint32Big(0);

    writer.writeUint32Big((atUint32)m_nameEntries.size());
    for (const NameEntry& entry : m_nameEntries)
    {
        NameEntry copy = entry;
        copy.nameLen = copy.name.size();
        copy.write(writer);
    }

    writer.writeUint32Big(m_entries.size());
    for (const auto& entry : m_entries)
    {
        Entry tmp = entry.second;
        if (tmp.compressed)
            tmp.compressed = 1;
        tmp.write(writer);
    }
}

template <>
void PAK::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s)
{
    s += 12;

    for (const NameEntry& entry : m_nameEntries)
        s += 12 + entry.name.size();

    s += m_entries.size() * 20 + 4;
}

std::unique_ptr<atUint8[]>
PAK::Entry::getBuffer(const nod::Node& pak, atUint64& szOut) const
{
    if (compressed)
    {
        std::unique_ptr<nod::IPartReadStream> strm = pak.beginReadStream(offset);

        atUint32 decompSz;
        strm->read(&decompSz, 4);
        decompSz = hecl::SBig(decompSz);
        atUint8* buf = new atUint8[decompSz];
        atUint8* bufCur = buf;

        atUint8 compBuf[0x8000];
        if (compressed == 1)
        {
            atUint32 compRem = size - 4;
            z_stream zs = {};
            inflateInit(&zs);
            zs.avail_out = decompSz;
            zs.next_out = buf;
            while (zs.avail_out)
            {
                atUint64 readSz = strm->read(compBuf, std::min(compRem, atUint32(0x8000)));
                compRem -= readSz;
                zs.avail_in = readSz;
                zs.next_in = compBuf;
                inflate(&zs, Z_FINISH);
            }
            inflateEnd(&zs);
        }
        else
        {
            atUint32 rem = decompSz;
            while (rem)
            {
                atUint16 chunkSz;
                strm->read(&chunkSz, 2);
                chunkSz = hecl::SBig(chunkSz);
                strm->read(compBuf, chunkSz);
                lzo_uint dsz = rem;
                lzo1x_decompress(compBuf, chunkSz, bufCur, &dsz, nullptr);
                bufCur += dsz;
                rem -= dsz;
            }
        }

        szOut = decompSz;
        return std::unique_ptr<atUint8[]>(buf);
    }
    else
    {
        atUint8* buf = new atUint8[size];
        pak.beginReadStream(offset)->read(buf, size);
        szOut = size;
        return std::unique_ptr<atUint8[]>(buf);
    }
}

const PAK::Entry* PAK::lookupEntry(const UniqueID32& id) const
{
    auto result = m_entries.find(id);
    if (result != m_entries.end())
        return &result->second;
    return nullptr;
}

const PAK::Entry* PAK::lookupEntry(std::string_view name) const
{
    auto result = m_nameMap.find(name.data());
    if (result != m_nameMap.end())
    {
        auto result1 = m_entries.find(result->second);
        if (result1 != m_entries.end())
            return &result1->second;
    }
    return nullptr;
}

std::string PAK::bestEntryName(const nod::Node& pakNode, const Entry& entry, bool& named) const
{
    std::unordered_map<UniqueID32, Entry>::const_iterator search;
    if (entry.type == FOURCC('AGSC') && (search = m_entries.find(entry.id)) != m_entries.cend())
    {
        /* Use internal AGSC name for entry */
        auto rs = search->second.beginReadStream(pakNode);
        AGSC::Header header;
        header.read(rs);
        named = true;
        return header.groupName;
    }

    /* Prefer named entries first */
    for (const NameEntry& nentry : m_nameEntries)
        if (nentry.id == entry.id)
        {
            named = true;
            return nentry.name;
        }

    /* Otherwise return ID format string */
    named = false;
    return entry.type.toString() + '_' + entry.id.toString();
}

}
