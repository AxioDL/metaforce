import bpy, sys, os, re, struct, traceback

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
    if 'TEMP' in os.environ:
        err_path = os.environ['TEMP']
else:
    err_path = "/tmp"
    if 'TMPDIR' in os.environ:
        err_path = os.environ['TMPDIR']

err_path += "/hecl_%016X.derp" % os.getpid()

def readpipestr():
    read_bytes = os.read(readfd, 4)
    if len(read_bytes) != 4:
        print('HECL connection lost or desynchronized')
        bpy.ops.wm.quit_blender()
    read_len = struct.unpack('I', read_bytes)[0]
    return os.read(readfd, read_len)

def writepipestr(linebytes):
    #print('LINE', linebytes)
    os.write(writefd, struct.pack('I', len(linebytes)))
    os.write(writefd, linebytes)

def writepipebuf(linebytes):
    #print('BUF', linebytes)
    os.write(writefd, linebytes)

def quitblender():
    writepipestr(b'QUITTING')
    bpy.ops.wm.quit_blender()

class PathHasher:
    def hashpath32(self, path):
        writepipestr(path.encode())
        read_str = readpipestr()
        if len(read_str) >= 16:
            hash = int(read_str[0:16], 16)
            return (hash & 0xffffffff) ^ ((hash >> 32) & 0xffffffff)
        return 0

    def hashpath64(self, path):
        writepipestr(path.encode())
        read_str = readpipestr()
        if len(read_str) >= 16:
            return int(read_str[0:16], 16)
        return 0

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
    writepipestr(b'NOADDON')
    bpy.ops.wm.quit_blender()

# Quit if just installed
if did_install:
    writepipestr(b'ADDONINSTALLED')
    bpy.ops.wm.quit_blender()

# Intro handshake
writepipestr(b'READY')
ackbytes = readpipestr()
if ackbytes != b'ACK':
    quitblender()

# slerp branch check
bpy.ops.mesh.primitive_cube_add()
orig_rot = bpy.context.object.rotation_mode
try:
    bpy.context.object.rotation_mode = 'QUATERNION_SLERP'
    writepipestr(b'SLERP1')
except:
    writepipestr(b'SLERP0')
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
    cmdline = readpipestr()
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
    try:
        co = compile(compbuf, '<HECL>', 'exec')
        exec(co, globals)
    except Exception as e:
        trace_prefix = 'Error processing:\n'
        trace_prefix += compbuf
        raise RuntimeError(trace_prefix) from e

# Command loop for writing animation key data to blender
def animin_loop(globals):
    writepipestr(b'ANIMREADY')
    while True:
        crv_type = struct.unpack('b', os.read(readfd, 1))
        if crv_type[0] < 0:
            writepipestr(b'ANIMDONE')
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

def writelight(obj):
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

    writepipestr(obj.name.encode())

# Command loop for reading data from blender
def dataout_loop():
    writepipestr(b'READY')
    while True:
        cmdargs = read_cmdargs()
        print(cmdargs)

        if cmdargs[0] == 'DATAEND':
            writepipestr(b'DONE')
            return

        elif cmdargs[0] == 'MESHLIST':
            meshCount = 0
            for meshobj in bpy.data.objects:
                if meshobj.type == 'MESH' and not meshobj.library:
                    meshCount += 1
            writepipebuf(struct.pack('I', meshCount))
            for meshobj in bpy.data.objects:
                if meshobj.type == 'MESH' and not meshobj.library:
                    writepipestr(meshobj.name.encode())

        elif cmdargs[0] == 'LIGHTLIST':
            lightCount = 0
            for obj in bpy.context.scene.objects:
                if obj.type == 'LAMP' and not obj.library:
                    lightCount += 1
            writepipebuf(struct.pack('I', lightCount))
            for obj in bpy.context.scene.objects:
                if obj.type == 'LAMP' and not obj.library:
                    writepipestr(obj.name.encode())

        elif cmdargs[0] == 'MESHAABB':
            writepipestr(b'OK')
            hecl.mesh_aabb(writepipebuf)

        elif cmdargs[0] == 'MESHCOMPILE':
            maxSkinBanks = int(cmdargs[2])

            meshName = bpy.context.scene.hecl_mesh_obj
            if meshName not in bpy.data.objects:
                writepipestr(('mesh %s not found' % meshName).encode())
                continue

            writepipestr(b'OK')
            hecl.hmdl.cook(writepipebuf, bpy.data.objects[meshName], cmdargs[1], maxSkinBanks)

        elif cmdargs[0] == 'MESHCOMPILENAME':
            meshName = cmdargs[1]
            maxSkinBanks = int(cmdargs[3])
            useLuv = int(cmdargs[4])

            if meshName not in bpy.data.objects:
                writepipestr(('mesh %s not found' % meshName).encode())
                continue

            writepipestr(b'OK')
            hecl.hmdl.cook(writepipebuf, bpy.data.objects[meshName], cmdargs[2], maxSkinBanks, useLuv)

        elif cmdargs[0] == 'MESHCOMPILENAMECOLLISION':
            meshName = cmdargs[1]

            if meshName not in bpy.data.objects:
                writepipestr(('mesh %s not found' % meshName).encode())
                continue

            writepipestr(b'OK')
            hecl.hmdl.cookcol(writepipebuf, bpy.data.objects[meshName])

        elif cmdargs[0] == 'MESHCOMPILECOLLISIONALL':
            writepipestr(b'OK')
            colCount = 0
            for obj in bpy.context.scene.objects:
                if obj.type == 'MESH' and not obj.library:
                    colCount += 1

            writepipebuf(struct.pack('I', colCount))

            for obj in bpy.context.scene.objects:
                if obj.type == 'MESH' and not obj.library:
                    hecl.hmdl.cookcol(writepipebuf, obj)

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

            writepipestr(b'OK')
            hecl.hmdl.cook(writepipebuf, join_obj, cmdargs[1], maxSkinBanks, maxOctantLength)

            bpy.context.scene.objects.unlink(join_obj)
            bpy.data.objects.remove(join_obj)
            bpy.data.meshes.remove(join_mesh)

        elif cmdargs[0] == 'MESHCOMPILEPATH':
            meshName = bpy.context.scene.hecl_path_obj
            if meshName not in bpy.data.objects:
                writepipestr(('mesh %s not found' % meshName).encode())
                continue

            writepipestr(b'OK')
            hecl.path.cook(writepipebuf, bpy.data.objects[meshName])

        elif cmdargs[0] == 'WORLDCOMPILE':
            writepipestr(b'OK')
            hecl.swld.cook(writepipebuf)

        elif cmdargs[0] == 'FRAMECOMPILE':
            version = int(cmdargs[1])
            if version != 0 and version != 1:
                writepipestr(b'bad version')
                continue

            writepipestr(b'OK')
            buffer = hecl.frme.cook(writepipebuf, version, PathHasher())
            writepipestr(b'FRAMEDONE')
            writepipebuf(struct.pack('I', len(buffer)))
            writepipebuf(buffer)

        elif cmdargs[0] == 'LIGHTCOMPILEALL':
            writepipestr(b'OK')
            lampCount = 0
            firstSpot = None
            for obj in bpy.context.scene.objects:
                if obj.type == 'LAMP':
                    lampCount += 1
                    if firstSpot is None and obj.data.type == 'SPOT':
                        firstSpot = obj

            # Ambient
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

            if firstSpot is not None:
                writelight(firstSpot)

            if ambient_energy:
                writepipebuf(struct.pack('ffffffffffffffff',
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0))
                writepipebuf(struct.pack('fff', ambient_color[0], ambient_color[1], ambient_color[2]))
                writepipebuf(struct.pack('IIfffffb', 0, 0, ambient_energy, 0.0, 1.0, 0.0, 0.0, False))
                writepipestr(b'AMBIENT')

            # Lamp objects
            for obj in bpy.context.scene.objects:
                if obj != firstSpot and obj.type == 'LAMP':
                    writelight(obj)

        elif cmdargs[0] == 'GETTEXTURES':
            writepipestr(b'OK')

            img_count = 0
            for img in bpy.data.images:
                if img.type == 'IMAGE':
                    img_count += 1
            writepipebuf(struct.pack('I', img_count))

            for img in bpy.data.images:
                if img.type == 'IMAGE':
                    path = os.path.normpath(bpy.path.abspath(img.filepath))
                    writepipebuf(struct.pack('I', len(path)))
                    writepipebuf(path.encode())

        elif cmdargs[0] == 'ACTORCOMPILE':
            writepipestr(b'OK')
            hecl.sact.cook(writepipebuf)

        elif cmdargs[0] == 'ACTORCOMPILECHARACTERONLY':
            writepipestr(b'OK')
            hecl.sact.cook_character_only(writepipebuf)

        elif cmdargs[0] == 'ACTIONCOMPILECHANNELSONLY':
            actionName = cmdargs[1]
            writepipestr(b'OK')
            hecl.sact.cook_action_channels_only(writepipebuf, actionName)

        elif cmdargs[0] == 'GETARMATURENAMES':
            writepipestr(b'OK')
            hecl.sact.get_armature_names(writepipebuf)

        elif cmdargs[0] == 'GETSUBTYPENAMES':
            writepipestr(b'OK')
            hecl.sact.get_subtype_names(writepipebuf)

        elif cmdargs[0] == 'GETSUBTYPEOVERLAYNAMES':
            subtypeName = cmdargs[1]
            writepipestr(b'OK')
            hecl.sact.get_subtype_overlay_names(writepipebuf, subtypeName)

        elif cmdargs[0] == 'GETACTIONNAMES':
            writepipestr(b'OK')
            hecl.sact.get_action_names(writepipebuf)

        elif cmdargs[0] == 'GETBONEMATRICES':
            armName = cmdargs[1]

            if armName not in bpy.data.objects:
                writepipestr(('armature %s not found' % armName).encode())
                continue

            armObj = bpy.data.objects[armName]
            if armObj.type != 'ARMATURE':
                writepipestr(('object %s not an ARMATURE' % armName).encode())
                continue

            writepipestr(b'OK')
            writepipebuf(struct.pack('I', len(armObj.data.bones)))
            for bone in armObj.data.bones:
                writepipebuf(struct.pack('I', len(bone.name)))
                writepipebuf(bone.name.encode())
                for r in bone.matrix_local.to_3x3():
                    for c in r:
                        writepipebuf(struct.pack('f', c))

        elif cmdargs[0] == 'RENDERPVS':
            pathOut = cmdargs[1]
            locX = float(cmdargs[2])
            locY = float(cmdargs[3])
            locZ = float(cmdargs[4])
            hecl.srea.render_pvs(pathOut, (locX, locY, locZ))
            writepipestr(b'OK')

        elif cmdargs[0] == 'RENDERPVSLIGHT':
            pathOut = cmdargs[1]
            lightName = cmdargs[2]
            hecl.srea.render_pvs_light(pathOut, lightName)
            writepipestr(b'OK')

        elif cmdargs[0] == 'MAPAREACOMPILE':
            if 'MAP' not in bpy.data.objects:
                writepipestr(('"MAP" object not in .blend').encode())
                continue
            map_obj = bpy.data.objects['MAP']
            if map_obj.type != 'MESH':
                writepipestr(('object "MAP" not a MESH').encode())
                continue
            writepipestr(b'OK')
            hecl.mapa.cook(writepipebuf, map_obj)

        elif cmdargs[0] == 'MAPUNIVERSECOMPILE':
            writepipestr(b'OK')
            hecl.mapu.cook(writepipebuf)

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
                writepipestr(b'FINISHED')
            else:
                writepipestr(b'CANCELLED')

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
                writepipestr(b'FINISHED')
            else:
                writepipestr(b'CANCELLED')

        elif cmdargs[0] == 'GETTYPE':
            writepipestr(bpy.context.scene.hecl_type.encode())

        elif cmdargs[0] == 'GETMESHRIGGED':
            meshName = bpy.context.scene.hecl_mesh_obj
            if meshName not in bpy.data.objects:
                writepipestr(b'FALSE')
            else:
                if len(bpy.data.objects[meshName].vertex_groups):
                    writepipestr(b'TRUE')
                else:
                    writepipestr(b'FALSE')

        elif cmdargs[0] == 'SAVE':
            bpy.context.user_preferences.filepaths.save_version = 0
            print('SAVING %s' % loaded_blend)
            if loaded_blend:
                if 'FINISHED' in bpy.ops.wm.save_as_mainfile(filepath=loaded_blend, check_existing=False, compress=True):
                    writepipestr(b'FINISHED')
                else:
                    writepipestr(b'CANCELLED')

        elif cmdargs[0] == 'PYBEGIN':
            writepipestr(b'READY')
            globals = {'hecl':hecl}
            compbuf = str()
            bracket_count = 0
            while True:
                try:
                    line = readpipestr()

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
                        writepipestr(b'DONE')
                        break

                    # Syntax filter
                    linestr = line.decode().rstrip()
                    if not len(linestr) or linestr.lstrip()[0] == '#':
                        writepipestr(b'OK')
                        continue
                    leading_spaces = len(linestr) - len(linestr.lstrip())

                    # Block lines always get appended right away
                    if linestr.endswith(':') or leading_spaces or bracket_count:
                        if len(compbuf):
                            compbuf += '\n'
                        compbuf += linestr
                        bracket_count += count_brackets(linestr)
                        writepipestr(b'OK')
                        continue

                    # Complete non-block statement in compbuf
                    if len(compbuf):
                        exec_compbuf(compbuf, globals)

                    # Establish new compbuf
                    compbuf = linestr
                    bracket_count += count_brackets(linestr)

                except Exception as e:
                    writepipestr(b'EXCEPTION')
                    raise
                    break
                writepipestr(b'OK')

        elif cmdargs[0] == 'PYEND':
            writepipestr(b'ERROR')

        elif cmdargs[0] == 'DATABEGIN':
            try:
                dataout_loop()
            except Exception as e:
                writepipestr(b'EXCEPTION')
                raise

        elif cmdargs[0] == 'DATAEND':
            writepipestr(b'ERROR')

        else:
            hecl.command(cmdargs, writepipestr, writepipebuf)

except Exception:
    fout = open(err_path, 'w')
    traceback.print_exc(file=fout)
    fout.close()
    raise
