import bpy, sys, os

# Extract pipe file descriptors from arguments
print(sys.argv)
if '--' not in sys.argv:
    bpy.ops.wm.quit_blender()
args = sys.argv[sys.argv.index('--')+1:]
readfd = int(args[0])
writefd = int(args[1])
print('READ', readfd, 'WRITE', writefd)

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

# Check that HECL addon is installed/enabled
if 'hecl' not in bpy.context.user_preferences.addons:
    if 'FINISHED' not in bpy.ops.wm.addon_enable(module='hecl'):
        writepipeline(b'NOADDON')
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
        bpy.ops.wm.open_mainfile(filepath=cmdline[1].decode())
        writepipeline(b'SUCCESS')

    elif cmdline[0] == b'TYPE':
        objname = cmdline[1].decode()

    else:
        writepipeline(b'RESP ' + cmdline[0])

