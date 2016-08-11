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
err_path = ""
if sys.platform == "win32":
    import msvcrt
    readfd = msvcrt.open_osfhandle(readfd, os.O_RDONLY | os.O_BINARY)
    writefd = msvcrt.open_osfhandle(writefd, os.O_WRONLY | os.O_BINARY)
    err_path = "/Temp"
else:
    err_path = "/tmp"

if 'TMPDIR' in os.environ:
    err_path = os.environ['TMPDIR']

err_path += "/hecl_%016X.derp" % os.getpid()

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

# slerp branch check
orig_rot = bpy.context.object.rotation_mode
try:
    bpy.context.object.rotation_mode = 'QUATERNION_SLERP'
    writepipeline(b'SLERP1')
except:
    writepipeline(b'SLERP0')
bpy.context.object.rotation_mode = orig_rot

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
            for k in range(key_info[1]):
                key_data = struct.unpack('if', os.read(readfd, 8))
                pt = crv.keyframe_points[k]
                pt.interpolation = 'LINEAR'
                pt.co = (key_data[0], key_data[1])
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

        elif cmdargs[0] == 'MESHCOMPILENAMECOLLISION':
            meshName = cmdargs[1]

            if meshName not in bpy.data.objects:
                writepipeline(('mesh %s not found' % meshName).encode())
                continue

            writepipeline(b'OK')
            hecl.hmdl.cookcol(writepipebuf, bpy.data.objects[meshName])

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

        elif cmdargs[0] == 'LIGHTCOMPILEALL':
            writepipeline(b'OK')
            lampCount = 0;
            for obj in bpy.context.scene:
                if obj.type == 'LAMP':
                    lampCount += 1

            world = bpy.context.scene.world
            ambient_energy = 0.0
            ambient_color = None
            if world.use_nodes and 'Background' in world.node_tree.nodes:
                bg_node = world.node_tree.nodes['Background']
                ambient_energy = bg_node.inputs[1].default_value
                ambient_color = bg_node.inputs[0].default_value
                if ambient_energy:
                    lampCount += 1

            writepipebuf(struct.pack('I', lampCount))

            if ambient_energy:
                writepipebuf(struct.pack('ffffffffffffffff',
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0))
                writepipebuf(struct.pack('fff', ambient_color[0], ambient_color[1], ambient_color[2]))
                writepipebuf(struct.pack('IIfffffb', 0, 0, ambient_energy, 0.0, 1.0, 0.0, 0.0, False))

            for obj in bpy.context.scene:
                if obj.type == 'LAMP':
                    wmtx = obj.matrix_world
                    writepipebuf(struct.pack('ffffffffffffffff',
                    wmtx[0][0], wmtx[0][1], wmtx[0][2], wmtx[0][3],
                    wmtx[1][0], wmtx[1][1], wmtx[1][2], wmtx[1][3],
                    wmtx[2][0], wmtx[2][1], wmtx[2][2], wmtx[2][3],
                    wmtx[3][0], wmtx[3][1], wmtx[3][2], wmtx[3][3]))
                    writepipebuf(struct.pack('fff', obj.data.color[0], obj.data.color[1], obj.data.color[2]))

                    type = 2
                    spotCutoff = 0.0
                    hasFalloff = False
                    castShadow = False
                    if obj.data.type == 'POINT':
                        type = 2
                        hasFalloff = True
                        castShadow = obj.data.shadow_method != 'NOSHADOW'
                    elif obj.data.type == 'SPOT':
                        type = 3
                        hasFalloff = True
                        spotCutoff = obj.data.spot_size
                        castShadow = obj.data.shadow_method != 'NOSHADOW'
                    elif obj.data.type == 'SUN':
                        type = 1
                        castShadow = obj.data.shadow_method != 'NOSHADOW'

                    constant = 1.0
                    linear = 0.0
                    quadratic = 0.0
                    if hasFalloff:
                        if obj.data.falloff_type == 'INVERSE_COEFFICIENTS':
                            constant = obj.data.constant_coefficient
                            linear = obj.data.linear_coefficient
                            quadratic = obj.data.quadratic_coefficient

                    layer = 0
                    if 'retro_layer' in obj.data.keys():
                        layer = obj.data['retro_layer']

                    writepipebuf(struct.pack('IIfffffb', layer, type, obj.data.energy, spotCutoff, constant, linear, quadratic,
                                                         castShadow))

        elif cmdargs[0] == 'ACTORCOMPILE':
            writepipeline(b'OK')
            hecl.sact.cook(writepipebuf)

        elif cmdargs[0] == 'GETARMATURENAMES':
            writepipeline(b'OK')
            hecl.sact.get_armature_names(writepipebuf)

        elif cmdargs[0] == 'GETSUBTYPENAMES':
            writepipeline(b'OK')
            hecl.sact.get_subtype_names(writepipebuf)

        elif cmdargs[0] == 'GETACTIONNAMES':
            writepipeline(b'OK')
            hecl.sact.get_action_names(writepipebuf)

        elif cmdargs[0] == 'GETBONEMATRICES':
            armName = cmdargs[1]

            if armName not in bpy.data.objects:
                writepipeline(('armature %s not found' % armName).encode())
                continue

            armObj = bpy.data.objects[armName]
            if armObj.type != 'ARMATURE':
                writepipeline(('object %s not an ARMATURE' % armName).encode())
                continue

            writepipeline(b'OK')
            writepipebuf(struct.pack('I', len(armObj.data.bones)))
            for bone in armObj.data.bones:
                writepipebuf(struct.pack('I', len(bone.name)))
                writepipebuf(bone.name.encode())
                for r in bone.matrix_local.to_3x3():
                    for c in r:
                        writepipebuf(struct.pack('f', c))

loaded_blend = None

# Main exception handling
try:
    # Command loop
    while True:
        cmdargs = read_cmdargs()
        print(cmdargs)

        if cmdargs[0] == 'QUIT':
            quitblender()

        elif cmdargs[0] == 'OPEN':
            if 'FINISHED' in bpy.ops.wm.open_mainfile(filepath=cmdargs[1]):
                if bpy.ops.object.mode_set.poll():
                    bpy.ops.object.mode_set(mode = 'OBJECT')
                loaded_blend = cmdargs[1]
                writepipeline(b'FINISHED')
            else:
                writepipeline(b'CANCELLED')

        elif cmdargs[0] == 'CREATE':
            if len(cmdargs) >= 4:
                bpy.ops.wm.open_mainfile(filepath=cmdargs[3])
                loaded_blend = cmdargs[1]
            else:
                bpy.ops.wm.read_homefile()
                loaded_blend = None
            bpy.context.user_preferences.filepaths.save_version = 0
            if 'FINISHED' in bpy.ops.wm.save_as_mainfile(filepath=cmdargs[1]):
                bpy.ops.file.hecl_patching_load()
                bpy.context.scene.hecl_type = cmdargs[2]
                writepipeline(b'FINISHED')
            else:
                writepipeline(b'CANCELLED')

        elif cmdargs[0] == 'GETTYPE':
            writepipeline(bpy.context.scene.hecl_type.encode())

        elif cmdargs[0] == 'GETMESHRIGGED':
            meshName = bpy.context.scene.hecl_mesh_obj
            if meshName not in bpy.data.objects:
                writepipeline(b'FALSE')
            else:
                if len(bpy.data.objects[meshName].vertex_groups):
                    writepipeline(b'TRUE')
                else:
                    writepipeline(b'FALSE')

        elif cmdargs[0] == 'SAVE':
            bpy.context.user_preferences.filepaths.save_version = 0
            if loaded_blend:
                if 'FINISHED' in bpy.ops.wm.save_as_mainfile(filepath=loaded_blend, check_existing=False, compress=True):
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

except Exception:
    import traceback
    fout = open(err_path, 'w')
    traceback.print_exc(file=fout)
    fout.close()
    raise
