import bpy, sys, os, re

ARGS_PATTERN = re.compile(r'''(?:"([^"]+)"|'([^']+)'|(\S+))''')

# Extract pipe file descriptors from arguments
print(sys.argv)
if '--' not in sys.argv:
    bpy.ops.wm.quit_blender()
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
    os.write(writefd, linebytes + b'\n')

def writepipebuf(linebytes):
    writepipeline(b'BUF')
    os.write(writefd, struct.pack('I', len(linebytes)) + linebytes)

def quitblender():
    writepipeline(b'QUITTING')
    bpy.ops.wm.quit_blender()

# Check that HECL addon is installed/enabled
if 'hecl' not in bpy.context.user_preferences.addons:
    if 'FINISHED' not in bpy.ops.wm.addon_enable(module='hecl'):
        writepipeline(b'NOADDON')
        bpy.ops.wm.quit_blender()

# Make addon available to commands
import hecl

# Intro handshake
writepipeline(b'READY')
ackbytes = readpipeline()
if ackbytes != b'ACK':
    quitblender()

# Command loop
while True:
    cmdline = readpipeline()
    if cmdline == b'':
        print('HECL connection lost')
        bpy.ops.wm.quit_blender()
    cmdargs = []
    for match in ARGS_PATTERN.finditer(cmdline.decode()):
        cmdargs.append(match.group(match.lastindex))
    print(cmdargs)

    if cmdargs[0] == 'QUIT':
        quitblender()

    elif cmdargs[0] == 'OPEN':
        if 'FINISHED' in bpy.ops.wm.open_mainfile(filepath=cmdargs[1]):
            writepipeline(b'FINISHED')
        else:
            writepipeline(b'CANCELLED')

    elif cmdargs[0] == 'CREATE':
        bpy.context.user_preferences.filepaths.save_version = 0
        if 'FINISHED' in bpy.ops.wm.save_as_mainfile(filepath=cmdargs[1]):
            writepipeline(b'FINISHED')
        else:
            writepipeline(b'CANCELLED')

    elif cmdargs[0] == 'PYBEGIN':
        writepipeline(b'READY')
        globals = dict()
        locals = dict()
        while True:
            try:
                line = readpipeline()
                if line == b'PYEND':
                    writepipeline(b'DONE')
                    break
                co = compile(line+b'\n', '<HECL>', 'single')
                exec(co, globals, locals)
            except Exception as e:
                writepipeline(b'EXCEPTION')
                raise
                break
            writepipeline(b'OK')

    elif cmdargs[0] == 'PYEND':
        writepipeline(b'ERROR')

    else:
        hecl.command(cmdargs, writepipeline, writepipebuf)

