import bpy, sys, os, re, code

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

    elif cmdargs[0] == 'SAVE':
        bpy.context.user_preferences.filepaths.save_version = 0
        if 'FINISHED' in bpy.ops.wm.save_mainfile(check_existing=False):
            writepipeline(b'FINISHED')
        else:
            writepipeline(b'CANCELLED')

    elif cmdargs[0] == 'PYBEGIN':
        writepipeline(b'READY')
        globals = dict()
        compbuf = str()
        prev_leading_spaces = 0
        while True:
            try:
                line = readpipeline()
                if line == b'PYEND':
                    writepipeline(b'DONE')
                    break
                linestr = line.decode()
                if linestr.isspace() or not len(linestr):
                    writepipeline(b'OK')
                    continue
                leading_spaces = len(linestr) - len(linestr.lstrip())
                if prev_leading_spaces and not leading_spaces:
                    compbuf += '\n'
                    co = code.compile_command(compbuf, filename='<HECL>')
                    if co is not None:
                        exec(co, globals)
                        compbuf = str()
                prev_leading_spaces = leading_spaces
                if len(compbuf):
                    compbuf += '\n'
                compbuf += linestr
                co = code.compile_command(compbuf, filename='<HECL>')
                if co is not None:
                    exec(co, globals)
                    compbuf = str()
            except Exception as e:
                writepipeline(b'EXCEPTION')
                raise
                break
            writepipeline(b'OK')

    elif cmdargs[0] == 'PYEND':
        writepipeline(b'ERROR')

    else:
        hecl.command(cmdargs, writepipeline, writepipebuf)

