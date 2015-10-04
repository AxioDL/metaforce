'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>

Traces the 'Blender Internal' shader node structure to generate a
HECL combiner string
'''

import bpy, bpy.path, os.path

def get_texmap_idx(tex_list, name):
    for i in range(len(tex_list)):
        if tex_list[i] == name:
            return i
    retval = len(tex_list)
    tex_list.append(name)
    return retval

def get_texture_path(name):
    if name not in bpy.data.textures:
        raise RuntimeError('unable to find %s texture' % name)
    tex = bpy.data.textures[name]
    if tex.type != 'IMAGE':
        raise RuntimeError('%s texture unsupported for %s, please save as IMAGE' % (tex.type, name))
    img = tex.image
    if not img:
        raise RuntimeError('image not set in %s' % name)
    return os.path.normpath(bpy.path.abspath(img.filepath))

# Trace color node structure
def recursive_color_trace(mat_obj, mesh_obj, tex_list, node, socket=None):

    if node.type == 'OUTPUT':
        if node.inputs['Color'].is_linked:
            return recursive_color_trace(mat_obj, mesh_obj, tex_list, node.inputs['Color'].links[0].from_node, node.inputs['Color'].links[0].from_socket)
        else:
            return 'vec3(%g,%g,%g)' % (node.inputs['Color'].default_value[0],
                                       node.inputs['Color'].default_value[1],
                                       node.inputs['Color'].default_value[2])

    elif node.type == 'MIX_RGB':
    
        if node.inputs[1].is_linked:
            a_input = recursive_color_trace(mat_obj, mesh_obj, tex_list, node.inputs[1].links[0].from_node, node.inputs[1].links[0].from_socket)
        else:
            a_input = 'vec3(%g,%g,%g)' % (node.inputs[1].default_value[0],
                                          node.inputs[1].default_value[1],
                                          node.inputs[1].default_value[2])

        if node.inputs[2].is_linked:
            b_input = recursive_color_trace(mat_obj, mesh_obj, tex_list, node.inputs[2].links[0].from_node, node.inputs[2].links[0].from_socket)
        else:
            b_input = 'vec3(%g,%g,%g)' % (node.inputs[2].default_value[0],
                                          node.inputs[2].default_value[1],
                                          node.inputs[2].default_value[2])

        if node.blend_type == 'MULTIPLY':
            return '(%s * %s)' % (a_input, b_input)
        elif node.blend_type == 'ADD':
            return '(%s + %s)' % (a_input, b_input)
        else:
            raise RuntimeError("HMDL does not support shaders with '{0}' blending modes".format(node.blend_type))

    elif node.type == 'TEXTURE':

        if not node.texture or not hasattr(node.texture, 'name'):
            raise RuntimeError("HMDL texture nodes must specify a texture object")

        if not node.inputs['Vector'].is_linked:
            raise RuntimeError("HMDL texture nodes must have a 'Geometry' or 'Group' UV modifier node linked")

        # Determine matrix generator type
        matrix_str = None
        soc_from = node.inputs['Vector'].links[0].from_socket

        if soc_from.node.type == 'GROUP':
            matrix_str = '%s(%%s, ' % soc_from.node.node_tree.name
            for s in range(len(soc_from.node.inputs)-1):
                soc = soc_from.node.inputs[s+1]
                if len(soc.links):
                    raise RuntimeError("UV Modifier nodes may not have parameter links (default values only)")
                ncomps = len(soc.default_value)
                if ncomps > 1:
                    matrix_str += 'vec%d(' % ncomps
                    for c in ncomps-1:
                        matrix_str += '%g, ' % soc.default_value[c]
                    matrix_str += '%g)' % soc.default_value[ncomps-1]
                else:
                    matrix_str += '%g' % soc.default_value

                if s == len(soc_from.node.inputs)-2:
                    matrix_str += ')'
                else:
                    matrix_str += ', '

            soc_from = soc_from.node.inputs[0].links[0].from_socket

        elif soc_from.node.type == 'GEOMETRY':
            pass

        else:
            raise RuntimeError("HMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")

        if soc_from.node.type != 'GEOMETRY':
            raise RuntimeError("Matrix animator nodes must connect to 'Geometry' node")


        # Resolve map and matrix index
        node_label = soc_from.node.label
        if not matrix_str and node_label.startswith('MTX_'):
            matrix_str = 'TexMtx(%%s, %d)' % int(node_label[4:])

        if soc_from.name == 'UV':
            uv_name = soc_from.node.uv_layer
            uv_idx = mesh_obj.data.uv_layers.find(uv_name)
            if uv_idx == -1:
                raise RuntimeError('UV Layer "%s" doesn\'t exist' % uv_name)
            uvsource_str = 'UV(%d)' % uv_idx

        elif soc_from.name == 'Normal':
            uvsource_str = 'Normal()'

        elif soc_from.name == 'View':
            uvsource_str = 'View()'

        else:
            raise RuntimeError("Only the 'UV', 'Normal' and 'View' sockets may be used from 'Geometry' nodes")

        if socket.name == 'Value':
            if matrix_str:
                uvsource_str = matrix_str % uvsource_str
            return 'texture(%d, %s).a' % (get_texmap_idx(tex_list, node.texture.name), uvsource_str)
        if socket.name == 'Color':
            if matrix_str:
                uvsource_str = matrix_str % uvsource_str
            return 'texture(%d, %s)' % (get_texmap_idx(tex_list, node.texture.name), uvsource_str)
        else:
            raise RuntimeError("Only the 'Value' or 'Color' output sockets may be used from Texture nodes")

    elif node.type == 'GROUP':

        group_str = '%s(' % node.node_tree.name
        did_first = False
        for input in node.inputs:
            if input.type == 'RGBA':
                if did_first:
                    group_str += ', '
                if input.is_linked:
                    group_str += recursive_color_trace(mat_obj, mesh_obj, tex_list, input.links[0].from_node, input.links[0].from_socket)
                else:
                    group_str += 'vec3(%g,%g,%g)' % (input.default_value[0],
                                                     input.default_value[1],
                                                     input.default_value[2])
                did_first = True
        group_str += ')'
        return group_str

    elif node.type == 'RGB':

        if node.label.startswith('DYNAMIC_'):
            dynamic_index = int(node.label[8:])
            return 'ColorReg(%d)' % dynamic_index

        return 'vec3(%g,%g,%g)' % (node.outputs['Color'].default_value[0],
                                   node.outputs['Color'].default_value[1],
                                   node.outputs['Color'].default_value[2])

    elif node.type == 'MATERIAL':
        
        if mat_obj.use_shadeless:
            return 'vec3(1.0)'
        else:
            return 'Lighting()'

    else:
        raise RuntimeError("HMDL is unable to process '{0}' shader nodes in '{1}'".format(node.type, mat_obj.name))



# Trace alpha node structure
def recursive_alpha_trace(mat_obj, mesh_obj, tex_list, node, socket=None):

    if node.type == 'OUTPUT':
        if node.inputs['Alpha'].is_linked:
            return recursive_alpha_trace(mat_obj, mesh_obj, tex_list, node.inputs['Alpha'].links[0].from_node, node.inputs['Alpha'].links[0].from_socket)
        else:
            return '%g' % node.inputs['Alpha'].default_value
    
    elif node.type == 'MATH':
    
        if node.inputs[0].is_linked:
            a_input = recursive_alpha_trace(mat_obj, mesh_obj, tex_list, node.inputs[0].links[0].from_node, node.inputs[0].links[0].from_socket)
        else:
            a_input = '%g' % node.inputs[0].default_value

        if node.inputs[1].is_linked:
            b_input = recursive_alpha_trace(mat_obj, mesh_obj, tex_list, node.inputs[1].links[0].from_node, node.inputs[1].links[0].from_socket)
        else:
            b_input = '%g' % node.inputs[1].default_value
        
        if node.operation == 'MULTIPLY':
            return '(%s * %s)' % (a_input, b_input)
        elif node.operation == 'ADD':
            return '(%s + %s)' % (a_input, b_input)
        else:
            raise RuntimeError("HMDL does not support shaders with '{0}' blending modes".format(node.operation))

    elif node.type == 'TEXTURE':

        if not node.texture or not hasattr(node.texture, 'name'):
            raise RuntimeError("HMDL texture nodes must specify a texture object")

        if not node.inputs['Vector'].is_linked:
            raise RuntimeError("HMDL texture nodes must have a 'Geometry' or 'Group' UV modifier node linked")
        
        # Determine matrix generator type
        matrix_str = None
        soc_from = node.inputs['Vector'].links[0].from_socket

        if soc_from.node.type == 'GROUP':
            matrix_str = '%s(%%s, ' % soc_from.node.node_tree.name
            for s in range(len(soc_from.node.inputs)-1):
                soc = soc_from.node.inputs[s+1]
                if len(soc.links):
                    raise RuntimeError("UV Modifier nodes may not have parameter links (default values only)")
                ncomps = len(soc.default_value)
                if ncomps > 1:
                    matrix_str += 'vec%d(' % ncomps
                    for c in ncomps-1:
                        matrix_str += '%g, ' % soc.default_value[c]
                    matrix_str += '%g)' % soc.default_value[ncomps-1]
                else:
                    matrix_str += '%g' % soc.default_value

                if s == len(soc_from.node.inputs)-2:
                    matrix_str += ')'
                else:
                    matrix_str += ', '

            soc_from = soc_from.node.inputs[0].links[0].from_socket

        elif soc_from.node.type == 'GEOMETRY':
            pass

        else:
            raise RuntimeError("HMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")

        if soc_from.node.type != 'GEOMETRY':
            raise RuntimeError("Matrix animator nodes must connect to 'Geometry' node")


        # Resolve map and matrix index
        node_label = soc_from.node.label
        if not matrix_str and node_label.startswith('MTX_'):
            matrix_str = 'TexMtx(%%s, %d)' % int(node_label[4:])

        if soc_from.name == 'UV':
            uv_name = soc_from.node.uv_layer
            uv_idx = mesh_obj.data.uv_layers.find(uv_name)
            if uv_idx == -1:
                raise RuntimeError('UV Layer "%s" doesn\'t exist' % uv_name)
            uvsource_str = 'UV(%d)' % uv_idx
            
        elif soc_from.name == 'Normal':
            uvsource_str = 'Normal()'

        elif soc_from.name == 'View':
            uvsource_str = 'View()'

        else:
            raise RuntimeError("Only the 'UV', 'Normal' and 'View' sockets may be used from 'Geometry' nodes")
        
        if socket.name == 'Value':
            if matrix_str:
                uvsource_str = matrix_str % uvsource_str
            return 'texture(%d, %s).a' % (get_texmap_idx(tex_list, node.texture.name), uvsource_str)
        else:
            raise RuntimeError("Only the 'Value' output sockets may be used from Texture nodes")

    elif node.type == 'GROUP':

        group_str = '%s(' % node.node_tree.name
        did_first = False
        for input in node.inputs:
            if input.type == 'VALUE':
                if did_first:
                    group_str += ', '
                if input.is_linked:
                    group_str += recursive_alpha_trace(mat_obj, mesh_obj, tex_list, input.links[0].from_node, input.links[0].from_socket)
                else:
                    group_str += '%g' % input.default_value
                did_first = True
        group_str += ')'
        return group_str

    elif node.type == 'VALUE':

        if node.label.startswith('DYNAMIC_'):
            dynamic_index = int(node.label[8:])
            return 'ColorReg(%d).a' % dynamic_index

        return '%g' % node.outputs['Value'].default_value

    elif node.type == 'MATERIAL':
    
        return '1.0'

    else:
        raise RuntimeError("HMDL is unable to process '{0}' shader nodes in '{1}'".format(node.type, mat_obj.name))



def shader(mat_obj, mesh_obj):

    if not mat_obj.use_nodes:
        raise RuntimeError("HMDL *requires* that shader nodes are used; '{0}' does not".format(mat_obj.name))

    if 'Output' not in mat_obj.node_tree.nodes or mat_obj.node_tree.nodes['Output'].type != 'OUTPUT':
        raise RuntimeError("HMDL *requires* that an OUTPUT shader node named 'Output' is present")

    # Root (output) node
    output_node = mat_obj.node_tree.nodes['Output']

    # Trace nodes and build result
    tex_list = []
    color_trace_result = recursive_color_trace(mat_obj, mesh_obj, tex_list, output_node)
    alpha_trace_result = recursive_alpha_trace(mat_obj, mesh_obj, tex_list, output_node)

    # Resolve texture paths
    tex_paths = [get_texture_path(name) for name in tex_list]

    if mat_obj.game_settings.alpha_blend == 'ALPHA' or mat_obj.game_settings.alpha_blend == 'ALPHA_SORT':
        return "HECLBlend(%s, %s)" % (color_trace_result, alpha_trace_result), tex_paths
    elif mat_obj.game_settings.alpha_blend == 'ADD':
        return "HECLAdditive(%s, %s)" % (color_trace_result, alpha_trace_result), tex_paths
    else:
        return "HECLOpaque(%s)" % color_trace_result, tex_paths

# DEBUG operator
import bpy
class hecl_shader_operator(bpy.types.Operator):
    bl_idname = "scene.hecl_shader"
    bl_label = "DEBUG HECL shader maker"
    bl_description = "Test shader generation utility"

    @classmethod
    def poll(cls, context):
        return context.object and context.object.type == 'MESH'

    def execute(self, context):
        shad, texs = shader(context.object.active_material, context.object, bpy.data.filepath)
        
        vs = bpy.data.texts.new('HECL SHADER')
        vs.write((shad + '\n'))
        for tex in texs:
            vs.write(tex + '\n')

        return {'FINISHED'}
