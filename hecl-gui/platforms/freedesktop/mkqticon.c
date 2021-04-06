#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static const int DIMS[] =
{
    16,
    32,
    48,
    64,
    128,
    256,
    0
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: makeqticon <out.bin>\n");
        return 1;
    }

    FILE* ofp = fopen(argv[1], "wb");
    if (!ofp)
    {
        fprintf(stderr, "'%s' is not able to be opened for writing as a regular file\n", argv[1]);
        return 1;
    }

    char command[2048];

    for (const int* d = DIMS ; *d != 0 ; ++d)
    {
        printf("Rendering main icon @%dx%d\n", *d, *d);
        fflush(stdout);

        snprintf(command, 2048, "%dx%d/apps/hecl.png", *d, *d);
        FILE* fp = fopen(command, "rb");
        if (!fp)
        {
            fprintf(stderr, "unable to open '%s' for reading\n", command);
            fclose(ofp);
            return 1;
        }

        fseek(fp, 0, SEEK_END);
        uint32_t size = (uint32_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);

        fwrite(&size, 1, 4, ofp);

        uint8_t buf[1024];
        while (size > 0)
        {
            long thisSize = size;
            if (thisSize > 1024)
                thisSize = 1024;

            fread(buf, 1, (size_t)thisSize, fp);
            fwrite(buf, 1, (size_t)thisSize, ofp);
            size -= thisSize;
        }

        fclose(fp);
    }

    fclose(ofp);
    return 0;
}
