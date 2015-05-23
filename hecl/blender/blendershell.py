import bpy, sys, os

# Extract pipe file descriptors from arguments
args = sys.argv[sys.argv.index('--')+1:]
readfd = int(args[0])
writefd = int(args[1])

def readpipeline():
    retval = bytearray()
    while True:
        ch = os.read(readfd, 1)
        if ch == b'\n' or ch == b'':
            return retval
        retval += ch

def writepipeline(linebytes):
    ch = os.write(writefd, linebytes + b'\n')

def quitblender():
    writepipeline(b'QUITTING')
    bpy.ops.wm.quit_blender()

# Intro handshake
writepipeline(b'READY')
ackbytes = readpipeline()
if ackbytes != b'ACK':
    quitblender()

# Command loop
while True:
    cmdline = readpipeline().split(b' ')

    if not len(cmdline) or cmdline[0] == b'QUIT':
        quitblender()

    elif cmdline[0] == b'OPEN':
        bpy.ops.wm.open_mainfile(filepath=cmdline[1].encode())
        writepipeline(b'SUCCESS')

    elif cmdline[0] == b'TYPE':
        objname = cmdline[1].encode()

