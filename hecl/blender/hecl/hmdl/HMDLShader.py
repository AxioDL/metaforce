import bpy, bpy.path, os.path, struct

def get_texture_path(image):
    return os.path.normpath(bpy.path.abspath(image.filepath))

SHADER_TYPES = {
    'RetroShader': b'RSHD',
    'RetroDynamicShader': b'RDYN',
    'RetroDynamicAlphaShader': b'RDAL',
    'RetroDynamicCharacterShader': b'RCHR',
}

PASS_TYPE = {
    'Lightmap': b'LMAP',
    'Diffuse': b'DIFF',
    'DiffuseMod': b'DIFM',
    'Emissive': b'EMIS',
    'Specular': b'SPEC',
    'ExtendedSpecular': b'ESPC',
    'Reflection': b'REFL',
    'IndirectTex': b'INDR',
    'Alpha': b'ALPH',
    'AlphaMod': b'ALPM'
}

def write_chunks(writebuf, mat_obj, mesh_obj):

    if not mat_obj.use_nodes:
        raise RuntimeError("HMDL *requires* that shader nodes are used; '{0}' does not".format(mat_obj.name))

    if 'Output' not in mat_obj.node_tree.nodes or mat_obj.node_tree.nodes['Output'].type != 'GROUP':
        raise RuntimeError("HMDL *requires* that an group shader node named 'Output' is present")

    # Root (output) node
    output_node = mat_obj.node_tree.nodes['Output']
    if output_node.node_tree.name not in SHADER_TYPES:
        raise RuntimeError("HMDL *requires* one of the RetroShader group nodes for the 'Output' node")
    writebuf(SHADER_TYPES[output_node.node_tree.name])

    # Count sockets
    chunk_count = 0
    for inp, def_inp in zip(output_node.inputs, output_node.node_tree.inputs):
        if inp.name in PASS_TYPE:
            if inp.is_linked:
                chunk_count += 1
            else:
                # Color pass
                color_set = False
                if inp.type == 'VALUE':
                    color_set = inp.default_value != def_inp.default_value
                else:
                    for comp, def_comp in zip(inp.default_value, def_inp.default_value):
                        color_set |= comp != def_comp
                if color_set:
                    chunk_count += 1

    writebuf(struct.pack('I', chunk_count))

    # Enumerate sockets
    for inp, def_inp in zip(output_node.inputs, output_node.node_tree.inputs):
        if inp.name in PASS_TYPE:
            pass_fourcc = PASS_TYPE[inp.name]
            if inp.is_linked:
                socket = inp.links[0].from_socket
                node = socket.node
                if node.type != 'TEX_IMAGE':
                    raise RuntimeError("HMDL requires all group node inputs connect to Image Texture nodes")

                if not node.image:
                    raise RuntimeError("HMDL texture nodes must specify an image object")

                if not node.inputs['Vector'].is_linked:
                    raise RuntimeError("HMDL texture nodes must have a 'Texture Coordinate', 'UV Map' or 'Group' UV modifier node linked")

                # Determine matrix generator type
                tex_coord_source = 0xff
                uv_anim_type = 0xff
                uv_anim_args = []
                soc_from = node.inputs['Vector'].links[0].from_socket

                if soc_from.node.type == 'GROUP':
                    if soc_from.node.node_tree.name.startswith('RetroUVMode'):
                        uv_anim_type = int(soc_from.node.node_tree.name[11:12])
                        if len(soc_from.node.inputs)-1:
                            for s in range(len(soc_from.node.inputs)-1):
                                soc = soc_from.node.inputs[s+1]
                                if len(soc.links):
                                    raise RuntimeError("UV Modifier nodes may not have parameter links (default values only)")
                                if soc.type == 'VALUE':
                                    uv_anim_args.append(soc.default_value)
                                else:
                                    uv_anim_args.append(soc.default_value[0])
                                    uv_anim_args.append(soc.default_value[1])
                    soc_from = soc_from.node.inputs[0].links[0].from_socket

                elif soc_from.node.type == 'UVMAP' or soc_from.node.type == 'TEX_COORD':
                    pass

                else:
                    raise RuntimeError("HMDL texture nodes must have a 'Texture Coordinate', 'UV Map' or 'Group' UV modifier node linked")

                if soc_from.node.type != 'UVMAP' and soc_from.node.type != 'TEX_COORD':
                    raise RuntimeError("Matrix animator nodes must connect to 'Texture Coordinate' or 'UV Map' node")


                # Resolve map and matrix index
                node_label = soc_from.node.label
                matrix_idx = None
                if node_label.startswith('MTX_'):
                    matrix_idx = int(node_label[4:])

                if soc_from.name == 'UV':
                    if hasattr(soc_from.node, 'uv_map'):
                        uv_name = soc_from.node.uv_map
                        uv_idx = mesh_obj.data.uv_layers.find(uv_name)
                        if uv_idx == -1:
                            raise RuntimeError('UV Layer "%s" doesn\'t exist' % uv_name)
                        tex_coord_source = uv_idx + 2
                    else:
                        tex_coord_source = 2

                elif soc_from.name == 'Normal':
                    tex_coord_source = 1

                elif soc_from.name == 'Window':
                    tex_coord_source = 0

                else:
                    raise RuntimeError("Only the 'UV', 'Normal' and 'Window' sockets may be used from 'Texture Coordinate' nodes")

                alpha = False
                if socket.name == 'Alpha':
                    alpha = True

                writebuf(b'PASS')
                writebuf(pass_fourcc)
                path = get_texture_path(node.image)
                writebuf(struct.pack('I', len(path)))
                writebuf(path.encode())
                writebuf(struct.pack('B', tex_coord_source))
                writebuf(struct.pack('B', uv_anim_type))
                writebuf(struct.pack('I', len(uv_anim_args)))
                for arg in uv_anim_args:
                    writebuf(struct.pack('f', arg))
                writebuf(struct.pack('B', alpha))

            else:
                # Color pass
                color_set = False
                if inp.type == 'VALUE':
                    color_set = inp.default_value != def_inp.default_value
                else:
                    for comp, def_comp in zip(inp.default_value, def_inp.default_value):
                        color_set |= comp != def_comp

                if color_set:
                    writebuf(b'CLR ')
                    writebuf(pass_fourcc)
                    if inp.type == 'VALUE':
                        writebuf(struct.pack('ffff', inp.default_value, inp.default_value,
                                             inp.default_value, inp.default_value))
                    else:
                        writebuf(struct.pack('ffff', inp.default_value[0], inp.default_value[1],
                                             inp.default_value[2], inp.default_value[3]))
