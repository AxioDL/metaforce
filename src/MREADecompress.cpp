#include "RetroCommon.hpp"
#include <Athena/Compression.hpp>
#include <memory.h>

enum MREAVersion
{
    MetroidPrimeDemo = 0xC,
    MetroidPrime1 = 0x0F,
    MetroidPrime2 = 0x19,
    MetroidPrime3 = 0x1E,
    DKCR          = 0x20
};


struct CompressedBlockInfo
{
    atUint32 blockSize;
    atUint32 dataSize;
    atUint32 dataCompSize;
    atUint32 sectionCount; // How many actual sections specified by header.sectionCount are in this compressed bock
};


bool decompressBlock(CompressedBlockInfo& info, Athena::io::IStreamReader& in, Athena::io::IStreamWriter& out);

bool decompressMREA(Athena::io::IStreamReader& in, Athena::io::IStreamWriter& out)
{
    try
    {
        // Do this as a precaution, MREAs are always in big endian
        in.setEndian(Athena::Endian::BigEndian);
        out.setEndian(Athena::Endian::BigEndian);
        atUint32 magic = in.readUint32();

        if (magic != 0xDEADBEEF)
            return false;

        atUint32 version = in.readUint32();

        // Metroid prime 1 MREAs aren't compressed
        if (version == MetroidPrime1 || version == MetroidPrimeDemo)
            return false;

        out.writeUint32(magic);
        out.writeUint32(version);
        
        atInt8* mtxData = in.readBytes(sizeof(float[3][4]));
        out.writeBytes(mtxData, sizeof(float[3][4]));
        delete[] mtxData;
        out.writeUint32(in.readUint32()); // mesh count
        out.writeUint32(in.readUint32()); // scly count

        atUint32 sectionCount = in.readUint32();
        out.writeUint32(sectionCount);

        if (version == MetroidPrime2)
        {
            for (atUint32 i = 0; i < 11; i++)
                out.writeUint32(in.readUint32());
        }

        atUint32 compressedBlockCount = in.readUint32();
        out.writeUint32(compressedBlockCount);
        atUint32 sectionNumberCount = 0;
        if (version == MetroidPrime3 || version == DKCR)
            sectionNumberCount = in.readUint32();
        out.writeUint32(sectionNumberCount);

        in.seekAlign32();
        out.seekAlign32();

        for (atUint32 i = 0; i < sectionCount; i++)
            out.writeUint32(in.readUint32());

        in.seekAlign32();
        out.seekAlign32();

        std::vector<CompressedBlockInfo> blockInfo;

        for (atUint32 i = 0; i < compressedBlockCount; i++)
        {
            CompressedBlockInfo block;
            block.blockSize    = in.readUint32();
            out.writeUint32(block.blockSize);
            block.dataSize     = in.readUint32();
            out.writeUint32(block.dataSize);
            block.dataCompSize = in.readUint32();
            out.writeUint32(0);
            block.sectionCount = in.readUint32();
            out.writeUint32(block.sectionCount);
            blockInfo.push_back(block);
        }

        // We only need to seek in, out is already where it needs to be
        in.seekAlign32();
        out.seekAlign32();

        if (version == MetroidPrime3 || version == DKCR)
        {
            for (atUint32 i = 0; i < sectionNumberCount * 2; i++)
                out.writeUint32(in.readUint32());

            in.seekAlign32();
            out.seekAlign32();
        }

        for (CompressedBlockInfo info : blockInfo)
            if (!decompressBlock(info, in, out))
                return false;
    }
    catch(...)
    {
        return false;
    }

    return true;
}

bool decompressBlock(CompressedBlockInfo& info, Athena::io::IStreamReader& in, Athena::io::IStreamWriter& out)
{
    try
    {
        // if dataCompSize is 0 we just write the raw data
        if (info.dataCompSize == 0)
        {
            atUint8* rawData = in.readUBytes(info.dataSize);
            out.writeUBytes(rawData, info.dataSize);
            delete[] rawData;
        }
        else
        {
            // We have compressed data, this is a bit tricky since the compression header isn't always located at the start of the data
            // Retro did something unorthodox, instead of padding the end of the block, they padded the beginning
            atUint32 blockStart = ROUND_UP_32(info.dataCompSize) - info.dataCompSize;

            atUint8* rawData = in.readUBytes(ROUND_UP_32(info.dataCompSize));
            atUint8* dataStart = rawData;
            rawData += blockStart;

            bool result = true;
            atUint32 decompressedSize = info.dataSize;
            atInt32 remainingSize = info.dataSize;

            // We use the blockSize because it's always larger than either size, it's also the behavior observed in the engine.
            atUint8* newData = new atUint8[info.blockSize];
            while (remainingSize > 0)
            {

                atUint16 segmentSize = *(atUint16*)(rawData);
                Athena::utility::BigUint16(segmentSize);
                rawData += 2;

                atUint16 peek = *(atUint16*)(rawData);
                Athena::utility::BigUint16(peek);
                if (peek != 0x78DA && peek != 0x7801 && peek != 0x789C)
                {
                    if (segmentSize > 0x4000)
                    {
                        // not compressed
                        memcpy(&newData[decompressedSize - remainingSize], rawData, 0x10000 - segmentSize);
                        rawData       += 0x10000 - segmentSize;
                        remainingSize -= 0x10000 - segmentSize;
                        result = true;
                        continue;
                    }

                    int lzoStatus = Athena::io::Compression::decompressLZO(rawData, segmentSize, &newData[decompressedSize - remainingSize], remainingSize);

                    if (!lzoStatus)
                        result = true;
                    else
                    {
                        result = false;
                        break;
                    }

                    rawData += segmentSize;
                }
                else
                {

                    int err = Athena::io::Compression::decompressZlib(rawData, segmentSize, &newData[decompressedSize - remainingSize], decompressedSize);

                    if (err > 0)
                    {
                        remainingSize -= err;
                        result = true;
                    }
                    else
                    {
                        result = false;
                        break;
                    }

                    rawData += segmentSize;
                }
            }

            if (result)
                out.writeUBytes(newData, decompressedSize);

            delete[] newData;
            delete[] dataStart;
        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}
