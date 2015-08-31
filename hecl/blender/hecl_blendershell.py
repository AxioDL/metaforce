import bpy, sys, os, re

ARGS_PATTERN = re.compile(r'''(?:"([^"]+)"|'([^']+)'|(\S+))''')

# Extract pipe file descriptors from arguments
print('HECL Blender Launch', sys.argv)
if '--' not in sys.argv:
    bpy.ops.wm.quit_blender()
args = sys.argv[sys.argv.index('--')+1:]
readfd = int(args[0])
writefd = int(args[1])
if sys.platform == "win32":
    import msvcrt
    readfd = msvcrt.open_osfhandle(readfd, os.O_RDONLY | os.O_BINARY)
    writefd = msvcrt.open_osfhandle(writefd, os.O_WRONLY | os.O_BINARY)

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

# If there's a third argument, use it as the .zip path containing the addon
if len(args) >= 3:
    bpy.ops.wm.addon_install(overwrite=True, target='DEFAULT', filepath=args[2])
    bpy.ops.wm.addon_refresh()

# Make addon available to commands
if bpy.context.user_preferences.addons.find('hecl') == -1:
    try:
        bpy.ops.wm.addon_enable(module='hecl')
        bpy.ops.wm.save_userpref()
    except:
        pass
try:
    import hecl
except:
    writepipeline(b'NOADDON')
    bpy.ops.wm.quit_blender()

# Intro handshake
writepipeline(b'READY')
ackbytes = readpipeline()
if ackbytes != b'ACK':
    quitblender()

# Count brackets
def count_brackets(linestr):
    bracket_count = 0
    for ch in linestr:
        if ch in {'[','{','('}:
            bracket_count += 1
        elif ch in {']','}',')'}:
            bracket_count -= 1
    return bracket_count

# Complete sequences of statements compiled/executed here
def exec_compbuf(compbuf, globals):
    #print('EXEC', compbuf)
    co = compile(compbuf, '<HECL>', 'exec')
    exec(co, globals)

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
        bpy.ops.wm.read_homefile()
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
        globals = {'hecl':hecl}
        compbuf = str()
        bracket_count = 0
        while True:
            try:
                line = readpipeline()

                # End check
                if line == b'PYEND':
                    # Ensure remaining block gets executed
                    if len(compbuf):
                        exec_compbuf(compbuf, globals)
                        compbuf = str()
                    writepipeline(b'DONE')
                    break

                # Syntax filter
                linestr = line.decode().rstrip()
                if not len(linestr) or linestr.lstrip()[0] == '#':
                    writepipeline(b'OK')
                    continue
                leading_spaces = len(linestr) - len(linestr.lstrip())

                # Block lines always get appended right away
                if linestr.endswith(':') or leading_spaces or bracket_count:
                    if len(compbuf):
                        compbuf += '\n'
                    compbuf += linestr
                    bracket_count += count_brackets(linestr)
                    writepipeline(b'OK')
                    continue

                # Complete non-block statement in compbuf
                if len(compbuf):
                    exec_compbuf(compbuf, globals)

                # Establish new compbuf
                compbuf = linestr
                bracket_count += count_brackets(linestr)

            except Exception as e:
                writepipeline(b'EXCEPTION')
                raise
                break
            writepipeline(b'OK')

    elif cmdargs[0] == 'PYEND':
        writepipeline(b'ERROR')

    else:
        hecl.command(cmdargs, writepipeline, writepipebuf)

