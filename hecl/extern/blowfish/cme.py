import struct

P = []
S = []

def F(x):
    d = x & 0x00FF
    x >>= 8
    c = x & 0x00FF
    x >>= 8
    b = x & 0x00FF
    x >>= 8
    a = x & 0x00FF
    #y = ((S[0][a] + S[1][b]) ^ S[2][c]) + S[3][d];
    y = S[0][a] + S[1][b]
    y = y ^ S[2][c]
    y = y + S[3][d]

    return y

def bfencipher(xl, xr):
    Xl = xl
    Xr = xr

    for i in range(16):
        Xl = Xl ^ P[i]
        Xr = F(Xl) ^ Xr

        temp = Xl
        Xl = Xr
        Xr = temp

    temp = Xl
    Xl = Xr
    Xr = temp

    Xr = Xr ^ P[16]
    Xl = Xl ^ P[16+1]

    return Xl&0xffffffff, Xr&0xffffffff

key = bytearray()
with open('hecl_key', 'r') as keyin:
    b = keyin.read(2)
    while len(b):
        if len(b) != 2:
            break
        key += struct.pack('B', int(b, 16))
        b = keyin.read(2)

if len(key) < 8:
    raise RuntimeError('key must be at least 8 bytes in length')

with open('blowfish.dat', 'rb') as fin:
    word = fin.read(4)
    for i in range(18):
        P.append(struct.unpack(">I", word)[0])
        word = fin.read(4)
    for i in range(4):
        sub = []
        for j in range(256):
            sub.append(struct.unpack(">I", word)[0])
            word = fin.read(4)
        S.append(sub)

j = 0
for i in range(18):
    data = 0x00000000
    for k in range(4):
        data = (data << 8) | key[j]
        j = j + 1
        if j >= len(key):
            j = 0
    P[i] = P[i] ^ data

datal = 0x00000000
datar = 0x00000000

for i in range(9):
    datal, datar = bfencipher(datal, datar)
    P[i*2] = datal
    P[i*2+1] = datar

for i in range(4):
    for j in range(128):
        datal, datar = bfencipher(datal, datar)
        S[i][j*2] = datal
        S[i][j*2+1] = datar

with open('blowfish.dat.c', 'w') as fout:
    fout.write('const unsigned long BLOWFISH_P[] = {')
    for i in range(18):
        if not i%4:
            fout.write('\n    ')
        fout.write('0x%08X,' % P[i])
    fout.write('\n};\n')
    fout.write('const unsigned long BLOWFISH_S[] = {') 
    for i in range(4):
        for j in range(256):
            if not (i*256+j)%4:
                fout.write('\n    ')
            fout.write('0x%08X,' % S[i][j])
    fout.write('\n};\n')

