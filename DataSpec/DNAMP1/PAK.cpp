#include <zlib.h>
#include <lzo/lzo1x.h>
#include "PAK.hpp"

namespace Retro
{
namespace DNAMP1
{

void PAK::read(Athena::io::IStreamReader& reader)
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
    }
    for (Entry& entry : m_entries)
        m_idMap[entry.id] = &entry;

    m_nameMap.clear();
    m_nameMap.reserve(nameCount);
    for (NameEntry& entry : m_nameEntries)
    {
        std::unordered_map<UniqueID32, Entry*>::iterator found = m_idMap.find(entry.id);
        if (found != m_idMap.end())
            m_nameMap[entry.name] = found->second;
    }
}

void PAK::write(Athena::io::IStreamWriter& writer) const
{
    writer.setEndian(Athena::BigEndian);
    writer.writeUint32(0x00030005);
    writer.writeUint32(0);

    writer.writeUint32(m_nameEntries.size());
    for (const NameEntry& entry : m_nameEntries)
    {
        ((NameEntry&)entry).nameLen = entry.name.size();
        entry.write(writer);
    }

    writer.writeUint32(m_entries.size());
    for (const Entry& entry : m_entries)
        entry.write(writer);
}

std::unique_ptr<atUint8[]> PAK::Entry::getBuffer(const NOD::DiscBase::IPartition::Node& pak, atUint64& szOut) const
{
    if (compressed)
    {
        std::unique_ptr<NOD::IPartReadStream> strm = pak.beginReadStream(offset);

        atUint32 decompSz;
        strm->read(&decompSz, 4);
        decompSz = HECL::SBig(decompSz);
        atUint8* buf = new atUint8[decompSz];
        atUint8* bufCur = buf;

        atUint16 zlibCheck;
        strm->read(&zlibCheck, 2);
        zlibCheck = HECL::SBig(zlibCheck);
        strm->seek(-2, SEEK_CUR);

        atUint8 compBuf[0x4000];
        if ((zlibCheck % 31) == 0)
        {
            atUint32 compRem = size - 4;
            z_stream zs;
            inflateInit(&zs);
            zs.avail_out = decompSz;
            zs.next_out = buf;
            while (zs.avail_out)
            {
                atUint64 readSz = strm->read(compBuf, MIN(compRem, 0x4000));
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
                chunkSz = HECL::SBig(chunkSz);
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

}
}
