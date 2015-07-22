#include "RetroCommon.hpp"
#include <Athena/Compression.hpp>
#include <memory.h>

struct CMPDBlock
{
    atUint32 compressedLen;
    atUint32 uncompressedLen;
};

void decompressData(aIO::IStreamWriter& outbuf,  const atUint8* srcData, atUint32 srcLength, atInt32 uncompressedLength)
{
    atUint16 compressionMethod = *(atUint16*)(srcData);
    Athena::utility::BigUint16(compressionMethod);
    if (compressionMethod == 0x78DA || compressionMethod == 0x7801 || compressionMethod == 0x789C)
    {
        atUint8*  decompData = new atUint8[uncompressedLength];
        if (aIO::Compression::decompressZlib(srcData, srcLength,  decompData, uncompressedLength) == uncompressedLength)
            outbuf.writeUBytes(decompData, uncompressedLength);
        delete[] decompData;
    }
    else
    {
        bool result = true;
        atUint8* newData = new atUint8[uncompressedLength];
        atInt32 remainingSize = uncompressedLength;
        do
        {
            if (remainingSize <= 0)
                break;

            atInt16 segmentSize = *(atInt16*)(srcData);
            srcData += 2;

            Athena::utility::BigInt16(segmentSize);

            if (segmentSize < 0)
            {
                segmentSize = -segmentSize;
                memcpy(&newData[uncompressedLength - remainingSize], srcData, segmentSize);
            }
            else
            {
                atInt32 lzoStatus = Athena::io::Compression::decompressLZO((const atUint8*)srcData, segmentSize, &newData[uncompressedLength - remainingSize], remainingSize);

                if ((lzoStatus & 8) != 0)
                {
                    result = false;
                    break;
                }
            }

            srcData  += segmentSize;
        }
        while (remainingSize > 0);

        if (result)
            outbuf.writeUBytes(newData, uncompressedLength);

        delete[] newData;
    }
}

void decompressFile(aIO::IStreamWriter& outbuf, const atUint8* data, atUint32 srcLength)
{
    atUint32 magic = *(atUint32*)(data);
    Athena::utility::BigUint32(magic);
    if (magic == 0x434D5044)
    {
        atUint32 currentOffset = 4;
        atUint32 blockCount = *(atUint32*)(data + currentOffset);
        currentOffset += 4;
        Athena::utility::BigUint32(blockCount);
        CMPDBlock* blocks = new CMPDBlock[blockCount];
        memcpy(blocks, data + currentOffset, sizeof(CMPDBlock)*blockCount);
        currentOffset += (sizeof(CMPDBlock)*blockCount);

        for (atUint32 i = 0; i < blockCount; i++)
        {
            Athena::utility::BigUint32(blocks[i].compressedLen);
            Athena::utility::BigUint32(blocks[i].uncompressedLen);

            blocks[i].compressedLen &= 0x00FFFFFF;

            if (blocks[i].compressedLen == blocks[i].uncompressedLen)
                outbuf.writeUBytes((atUint8*)(data + currentOffset), blocks[i].uncompressedLen);
            else
            {
                decompressData(outbuf, (const atUint8*)(data + currentOffset), blocks[i].compressedLen, blocks[i].uncompressedLen);
            }

            currentOffset += blocks[i].compressedLen;
        }
    }
    else
    {
        atUint32 uncompressedLength = *(atUint32*)(data);
        Athena::utility::BigUint32(uncompressedLength);
        atUint8* tmp = new atUint8[srcLength];
        memcpy(tmp, data + 4, srcLength - 4);
        decompressData(outbuf, (const atUint8*)tmp, srcLength - 4, uncompressedLength);
        delete[] tmp;
    }
}
