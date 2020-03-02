#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <zlib.h>

#define CHUNK 16384
#define LINE_BREAK 32
static uint8_t buf[CHUNK];
static uint8_t zbuf[CHUNK];

void print_usage() { fprintf(stderr, "Usage: bintoc [--compress] <in> <out> <symbol>\n"); }

int main(int argc, char** argv) {
  if (argc < 4) {
    print_usage();
    return 1;
  }
  char* input = argv[1];
  char* output = argv[2];
  char* symbol = argv[3];
  bool compress = false;
  if (strcmp(input, "--compress") == 0) {
    if (argc < 5) {
      print_usage();
      return 1;
    }
    input = argv[2];
    output = argv[3];
    symbol = argv[4];
    compress = true;
  }
  FILE* fin = fopen(input, "rb");
  if (!fin) {
    fprintf(stderr, "Unable to open %s for reading\n", input);
    return 1;
  }
  FILE* fout = fopen(output, "wb");
  if (!fout) {
    fprintf(stderr, "Unable to open %s for writing\n", output);
    return 1;
  }
  fprintf(fout, "#include <cstdint>\n#include <cstddef>\n");
  fprintf(fout, "extern \"C\" const uint8_t %s[] =\n{\n    ", symbol);
  size_t totalSz = 0;
  size_t readSz;
  if (compress) {
    size_t compressedSz = 0;
    z_stream strm = {.zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL};
    int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS | 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
      fprintf(stderr, "zlib initialization failed %d\n", ret);
      return 1;
    }
    while ((strm.avail_in = fread(buf, 1, sizeof(buf), fin))) {
      totalSz += strm.avail_in;
      strm.next_in = buf;
      int eof = feof(fin);
      do {
        strm.next_out = zbuf;
        strm.avail_out = sizeof(zbuf);
        ret = deflate(&strm, eof ? Z_FINISH : Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR) {
          fprintf(stderr, "zlib compression failed %d\n", ret);
          return 1;
        }
        size_t sz = sizeof(zbuf) - strm.avail_out;
        if (sz > 0) {
          for (int b = 0; b < sz; ++b) {
            fprintf(fout, "0x%02X, ", zbuf[b]);
            if ((compressedSz + b + 1) % LINE_BREAK == 0)
              fprintf(fout, "\n    ");
          }
          compressedSz += sz;
        }
      } while (strm.avail_out == 0 || (eof && (ret == Z_OK || ret == Z_BUF_ERROR)));
    }
    deflateEnd(&strm);
    fprintf(fout, "0x00};\nextern \"C\" const size_t %s_SZ = %zu;\n", symbol, compressedSz);
    fprintf(fout, "extern \"C\" const size_t %s_DECOMPRESSED_SZ = %zu;\n", symbol, totalSz);
  } else {
    while ((readSz = fread(buf, 1, sizeof(buf), fin))) {
      for (int b = 0; b < readSz; ++b) {
        fprintf(fout, "0x%02X, ", buf[b]);
        if ((totalSz + b + 1) % LINE_BREAK == 0)
          fprintf(fout, "\n    ");
      }
      totalSz += readSz;
    }
    fprintf(fout, "0x0};\nextern \"C\" const size_t %s_SZ = %zu;\n", symbol, totalSz);
  }
  fclose(fin);
  fclose(fout);
  return 0;
}
