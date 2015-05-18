import struct
with open('BLOWFISH.DAT', 'rb') as fin:
    with open('BLOWFISH.DAT.c', 'w') as fout:
        fout.write('#include <stdlib.h>\n')
        fin.seek(0, 2)
        sz = fin.tell()
        fout.write('size_t BLOWFISH_DAT_SZ = %u;\n' % sz)
        fout.write('char BLOWFISH_DAT[] = {')
        fin.seek(0)
        rb = fin.read(1)
        for i in range(sz):
            if not i%10:
                fout.write('\n    ')
            fout.write('0x%02X,' % struct.unpack("B", rb)[0])
            rb = fin.read(1)
        fout.write('\n0x0};\n')
