import bpy, sys, os, re, struct

ARGS_PATTERN = re.compile(r'''(?:"([^"]+)"|'([^']+)'|(\S+))''')

# Extract pipe file descriptors from arguments
print('HECL Blender Launch', sys.argv)
if '--' not in sys.argv:
    bpy.ops.wm.quit_blender()
args = sys.argv[sys.argv.index('--')+1:]
readfd = int(args[0])
writefd = int(args[1])
verbosity_level = int(args[2])
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
    #print('LINE', linebytes)
    os.write(writefd, linebytes + b'\n')

def writepipebuf(linebytes):
    #print('BUF', linebytes)
    os.write(writefd, linebytes)

def quitblender():
    writepipeline(b'QUITTING')
    bpy.ops.wm.quit_blender()

# If there's a third argument, use it as the .zip path containing the addon
did_install = False
if len(args) >= 4 and args[3] != 'SKIPINSTALL':
    bpy.ops.wm.addon_install(overwrite=True, target='DEFAULT', filepath=args[3])
    bpy.ops.wm.addon_refresh()
    did_install = True

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

# Quit if just installed
if did_install:
    writepipeline(b'ADDONINSTALLED')
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

# Read line of space-separated/quoted arguments
def read_cmdargs():
    cmdline = readpipeline()
    if cmdline == b'':
        print('HECL connection lost')
        bpy.ops.wm.quit_blender()
    cmdargs = []
    for match in ARGS_PATTERN.finditer(cmdline.decode()):
        cmdargs.append(match.group(match.lastindex))
    return cmdargs

# Complete sequences of statements compiled/executed here
def exec_compbuf(compbuf, globals):
    if verbosity_level >= 3:
        print('EXEC', compbuf)
    co = compile(compbuf, '<HECL>', 'exec')
    exec(co, globals)

# Command loop for writing animation key data to blender
def animin_loop(globals):
    writepipeline(b'ANIMREADY')
    while True:
        crv_type = struct.unpack('b', os.read(readfd, 1))
        if crv_type[0] < 0:
            writepipeline(b'ANIMDONE')
            return
        elif crv_type[0] == 0:
            crvs = globals['rotCurves']
        elif crv_type[0] == 1:
            crvs = globals['transCurves']
        elif crv_type[0] == 2:
            crvs = globals['scaleCurves']

        key_info = struct.unpack('ii', os.read(readfd, 8))
        crv = crvs[key_info[0]]
        crv.keyframe_points.add(count=key_info[1])

        if crv_type[0] == 1:
            trans_head = globals['bone_trans_head'][key_info[0]]
            for k in range(key_info[1]):
                key_data = struct.unpack('if', os.read(readfd, 8))
                pt = crv.keyframe_points[k]
                pt.interpolation = 'LINEAR'
                pt.co = (key_data[0], key_data[1] - trans_head)
        else:
            for k in range(key_info[1]):
                key_data = struct.unpack('if', os.read(readfd, 8))
                pt = crv.keyframe_points[k]
                pt.interpolation = 'LINEAR'
                pt.co = (key_data[0], key_data[1])

# Command loop for reading data from blender
def dataout_loop():
    writepipeline(b'READY')
    while True:
        cmdargs = read_cmdargs()
        print(cmdargs)

        if cmdargs[0] == 'DATAEND':
            writepipeline(b'DONE')
            return

        elif cmdargs[0] == 'MESHLIST':
            for meshobj in bpy.data.objects:
                if meshobj.type == 'MESH':
                    writepipeline(meshobj.name.encode())

        elif cmdargs[0] == 'MESHCOMPILE':
            maxSkinBanks = int(cmdargs[2])

            meshName = bpy.context.scene.hecl_mesh_obj
            if meshName not in bpy.data.objects:
                writepipeline(('mesh %s not found' % meshName).encode())
                continue

            writepipeline(b'OK')
            hecl.hmdl.cook(writepipebuf, bpy.data.objects[meshName], cmdargs[1], maxSkinBanks)

        elif cmdargs[0] == 'MESHCOMPILENAME':
            meshName = cmdargs[1]
            maxSkinBanks = int(cmdargs[3])

            if meshName not in bpy.data.objects:
                writepipeline(('mesh %s not found' % meshName).encode())
                continue

            writepipeline(b'OK')
            hecl.hmdl.cook(writepipebuf, bpy.data.objects[meshName], cmdargs[2], maxSkinBanks)

        elif cmdargs[0] == 'MESHCOMPILEALL':
            maxSkinBanks = int(cmdargs[2])
            maxOctantLength = float(cmdargs[3])

            bpy.ops.object.select_all(action='DESELECT')
            join_mesh = bpy.data.meshes.new('JOIN_MESH')
            join_obj = bpy.data.object.new(join_mesh.name, join_mesh)
            bpy.context.scene.objects.link(join_obj)
            bpy.ops.object.select_by_type(type='MESH')
            bpy.context.scene.objects.active = join_obj
            bpy.ops.object.join()

            writepipeline(b'OK')
            hecl.hmdl.cook(writepipebuf, join_obj, cmdargs[1], maxSkinBanks, maxOctantLength)

            bpy.context.scene.objects.unlink(join_obj)
            bpy.data.objects.remove(join_obj)
            bpy.data.meshes.remove(join_mesh)

        elif cmdargs[0] == 'ACTORCOMPILE':
            writepipeline(b'OK')
            hecl.sact.cook(writepipebuf)


# Command loop
while True:
    cmdargs = read_cmdargs()
    print(cmdargs)

    if cmdargs[0] == 'QUIT':
        quitblender()

    elif cmdargs[0] == 'OPEN':
        if 'FINISHED' in bpy.ops.wm.open_mainfile(filepath=cmdargs[1]):
            bpy.ops.object.mode_set(mode = 'OBJECT')
            writepipeline(b'FINISHED')
        else:
            writepipeline(b'CANCELLED')

    elif cmdargs[0] == 'CREATE':
        if len(cmdargs) >= 4:
            bpy.ops.wm.open_mainfile(filepath=cmdargs[3])
        else:
            bpy.ops.wm.read_homefile()
        bpy.context.user_preferences.filepaths.save_version = 0
        if 'FINISHED' in bpy.ops.wm.save_as_mainfile(filepath=cmdargs[1]):
            bpy.ops.file.hecl_patching_load()
            bpy.context.scene.hecl_type = cmdargs[2]
            writepipeline(b'FINISHED')
        else:
            writepipeline(b'CANCELLED')

    elif cmdargs[0] == 'GETTYPE':
        writepipeline(bpy.context.scene.hecl_type.encode())

    elif cmdargs[0] == 'SAVE':
        bpy.context.user_preferences.filepaths.save_version = 0
        if 'FINISHED' in bpy.ops.wm.save_mainfile(check_existing=False, compress=True):
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

                # ANIM check
                if line == b'PYANIM':
                    # Ensure remaining block gets executed
                    if len(compbuf):
                        exec_compbuf(compbuf, globals)
                        compbuf = str()
                    animin_loop(globals)
                    continue

                # End check
                elif line == b'PYEND':
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

    elif cmdargs[0] == 'DATABEGIN':
        try:
            dataout_loop()
        except Exception as e:
            writepipeline(b'EXCEPTION')
            raise

    elif cmdargs[0] == 'DATAEND':
        writepipeline(b'ERROR')

    else:
        hecl.command(cmdargs, writepipeline, writepipebuf)

