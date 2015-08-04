#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: bintoc <in> <out> <symbol>\n");
        return 1;
    }
    FILE* fin = fopen(argv[1], "rb");
    if (!fin)
    {
        fprintf(stderr, "Unable to open %s for reading\n", argv[1]);
        return 1;
    }
    FILE* fout = fopen(argv[2], "wb");
    if (!fout)
    {
        fprintf(stderr, "Unable to open %s for writing\n", argv[2]);
        return 1;
    }
    fprintf(fout, "#include <stdint.h>\n#include <stdlib.h>\nconst uint8_t %s[] =\n{\n", argv[3]);
    size_t totalSz = 0;
    size_t readSz;
    uint8_t buf[32];
    while ((readSz = fread(buf, 1, 32, fin)))
    {
        fprintf(fout, "    ");
        totalSz += readSz;
        for (int b=0 ; b<readSz ; ++b)
            fprintf(fout, "0x%02X, ", buf[b]);
        fprintf(fout, "\n");
    }
    fprintf(fout, "0x0};\nconst size_t %s_SZ = %zu;\n", argv[3], totalSz);
    fclose(fin);
    fclose(fout);
    return 0;
}
