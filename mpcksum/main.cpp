#include <cstdio>
#include "athena/MemoryReader.hpp"
#include "athena/FileWriter.hpp"
#include "athena/Checksums.hpp"

#if __BYTE_ORDER__ == __BIG_ENDIAN
#define SBIG(q) q
#else
#define SBIG(q) ( ( (q) & 0x000000FF ) << 24 | ( (q) & 0x0000FF00 ) <<  8 \
                | ( (q) & 0x00FF0000 ) >>  8 | ( (q) & 0xFF000000 ) >> 24 )
#endif

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("mpcksum <save> [output]\n");
        return 1;
    }

    athena::io::MemoryCopyReader in(argv[1]);
    std::unique_ptr<athena::io::FileWriter> out;
    if (argc > 2)
        out.reset(new athena::io::FileWriter(argv[2]));
    else
        out.reset(new athena::io::FileWriter(argv[1]));

    atUint32 magic;
    in.readUBytesToBuf(&magic, 4);
    if (magic != SBIG('GM8E') && magic != SBIG('GM8P') && magic != SBIG('GM8J'))
    {
        printf("Unsupported file, MP .gci file expected\n");
        return 1;
    }

    if (in.length() != 8256)
    {
        printf("File too small expected 8,256 bytes got %" PRIu64, in.length());
        return 1;
    }

    atUint8* data = in.data();

    atUint32 newCkSum = athena::checksums::crc32(data + 68, 8188, ~0, 0);
    *(atUint32*)(data + 64) = SBIG(newCkSum);
    out->writeBytes(data, 8256);

    delete data;

    return 0;
}
