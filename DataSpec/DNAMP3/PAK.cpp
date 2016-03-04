#include "PAK.hpp"
#include "DNAMP3.hpp"

namespace DataSpec
{
namespace DNAMP3
{

const hecl::FourCC CMPD("CMPD");

void PAK::read(athena::io::IStreamReader& reader)
{
    m_header.read(reader);
    if (m_header.version != 2)
        Log.report(logvisor::Fatal, "unexpected PAK magic");

    reader.seek(8, athena::Current);
    atUint32 strgSz = reader.readUint32Big();
    reader.seek(4, athena::Current);
    atUint32 rshdSz = reader.readUint32Big();
    reader.seek(44, athena::Current);
    atUint32 dataOffset = 128 + strgSz + rshdSz;

    atUint64 strgBase = reader.position();
    atUint32 nameCount = reader.readUint32Big();
    m_nameEntries.clear();
    m_nameEntries.reserve(nameCount);
    for (atUint32 n=0 ; n<nameCount ; ++n)
    {
        m_nameEntries.emplace_back();
        m_nameEntries.back().read(reader);
    }
    reader.seek(strgBase + strgSz, athena::Begin);

    atUint32 count = reader.readUint32Big();
    m_entries.clear();
    m_entries.reserve(count);
    m_firstEntries.clear();
    m_firstEntries.reserve(count);
    m_idMap.clear();
    m_idMap.reserve(count);
    for (atUint32 e=0 ; e<count ; ++e)
    {
        m_entries.emplace_back();
        m_entries.back().read(reader);
        m_entries.back().offset += dataOffset;
    }
    for (Entry& entry : m_entries)
    {
        auto search = m_idMap.find(entry.id);
        if (search == m_idMap.end())
        {
            m_firstEntries.push_back(&entry);
            m_idMap[entry.id] = &entry;
        }
    }

    m_nameMap.clear();
    m_nameMap.reserve(nameCount);
    for (NameEntry& entry : m_nameEntries)
    {
        auto search = m_idMap.find(entry.id);
        if (search != m_idMap.end())
        {
            m_nameMap[entry.name] = search->second;
            search->second->name = entry.name;
        }
    }
}
void PAK::write(athena::io::IStreamWriter& writer) const
{
    m_header.write(writer);

    DNAFourCC("STRG").write(writer);
    atUint32 strgSz = 4;
    for (const NameEntry& entry : m_nameEntries)
        strgSz += (atUint32)entry.name.size() + 13;
    atUint32 strgPad = ((strgSz + 63) & ~63) - strgSz;
    strgSz += strgPad;
    writer.writeUint32Big(strgSz);

    DNAFourCC("RSHD").write(writer);
    atUint32 rshdSz = 4 + 24 * m_entries.size();
    atUint32 rshdPad = ((rshdSz + 63) & ~63) - rshdSz;
    rshdSz += rshdPad;
    writer.writeUint32Big(rshdSz);
    atUint32 dataOffset = 128 + strgSz + rshdSz;

    DNAFourCC("DATA").write(writer);
    atUint32 dataSz = 0;
    for (const Entry& entry : m_entries)
        dataSz += (entry.size + 63) & ~63;
    atUint32 dataPad = ((dataSz + 63) & ~63) - dataSz;
    dataSz += dataPad;
    writer.writeUint32Big(dataSz);
    writer.seek(36, athena::Current);

    writer.writeUint32Big((atUint32)m_nameEntries.size());
    for (const NameEntry& entry : m_nameEntries)
        entry.write(writer);
    writer.seek(strgPad, athena::Current);

    writer.writeUint32Big((atUint32)m_entries.size());
    for (const Entry& entry : m_entries)
    {
        Entry copy = entry;
        copy.offset -= dataOffset;
        copy.write(writer);
    }
    writer.seek(rshdPad, athena::Current);
}
size_t PAK::binarySize(size_t __isz) const
{
    __isz = m_header.binarySize(__isz);

    size_t strgSz = 4;
    for (const NameEntry& entry : m_nameEntries)
        strgSz += entry.name.size() + 13;
    size_t strgPad = ((strgSz + 63) & ~63) - strgSz;

    size_t rshdSz = 4 + 24 * m_entries.size();
    size_t rshdPad = ((rshdSz + 63) & ~63) - rshdSz;

    __isz += 60;

    __isz += 4;
    for (const NameEntry& entry : m_nameEntries)
        __isz = entry.binarySize(__isz);
    __isz += strgPad;

    __isz += 4;
    for (const Entry& entry : m_entries)
        __isz = entry.binarySize(__isz);
    __isz += rshdPad;

    return __isz;
}

std::unique_ptr<atUint8[]> PAK::Entry::getBuffer(const nod::Node& pak, atUint64& szOut) const
{
    if (compressed)
    {
        std::unique_ptr<nod::IPartReadStream> strm = pak.beginReadStream(offset);
        struct
        {
            hecl::FourCC magic;
            atUint32 blockCount;
        } head;
        strm->read(&head, 8);
        if (head.magic != CMPD)
        {
            Log.report(logvisor::Error, "invalid CMPD block");
            return std::unique_ptr<atUint8[]>();
        }
        head.blockCount = hecl::SBig(head.blockCount);

        struct Block
        {
            atUint32 compSz;
            atUint32 decompSz;
        };
        std::unique_ptr<Block[]> blocks(new Block[head.blockCount]);
        strm->read(blocks.get(), 8 * head.blockCount);

        atUint64 maxBlockSz = 0;
        atUint64 totalDecompSz = 0;
        for (atUint32 b=0 ; b<head.blockCount ; ++b)
        {
            Block& block = blocks[b];
            block.compSz = hecl::SBig(block.compSz) & 0xffffff;
            block.decompSz = hecl::SBig(block.decompSz);
            if (block.compSz > maxBlockSz)
                maxBlockSz = block.compSz;
            totalDecompSz += block.decompSz;
        }

        std::unique_ptr<atUint8[]> compBuf(new atUint8[maxBlockSz]);
        atUint8* buf = new atUint8[totalDecompSz];
        atUint8* bufCur = buf;
        for (atUint32 b=0 ; b<head.blockCount ; ++b)
        {
            Block& block = blocks[b];
            atUint8* compBufCur = compBuf.get();
            strm->read(compBufCur, block.compSz);
            if (block.compSz == block.decompSz)
            {
                memcpy(bufCur, compBufCur, block.decompSz);
                bufCur += block.decompSz;
            }
            else
            {
                atUint32 rem = block.decompSz;
                while (rem)
                {
                    atUint16 chunkSz = hecl::SBig(*(atUint16*)compBufCur);
                    compBufCur += 2;
                    lzo_uint dsz = rem;
                    lzo1x_decompress(compBufCur, chunkSz, bufCur, &dsz, nullptr);
                    compBufCur += chunkSz;
                    bufCur += dsz;
                    rem -= dsz;
                }
            }
        }

        szOut = totalDecompSz;
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
