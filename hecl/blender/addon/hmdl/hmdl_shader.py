'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>

Traces the 'Blender Internal' shader node structure to generate a
HECL combiner string
'''

# Trace color node structure
def recursive_color_trace(mat_obj, mesh_obj, blend_path, node, socket=None):

    if node.type == 'OUTPUT':
        if node.inputs['Color'].is_linked:
            return recursive_color_trace(mat_obj, mesh_obj, blend_path, node.inputs['Color'].links[0].from_node, node.inputs['Color'].links[0].from_socket)
        else:
            return 'vec3(%f, %f, %f)' % (node.inputs['Color'].default_value[0],
                                         node.inputs['Color'].default_value[1],
                                         node.inputs['Color'].default_value[2])

    elif node.type == 'MIX_RGB':
    
        if node.inputs[1].is_linked:
            a_input = recursive_color_trace(mat_obj, mesh_obj, blend_path, node.inputs[1].links[0].from_node, node.inputs[1].links[0].from_socket)
        else:
            a_input = 'vec3(%f, %f, %f)' % (node.inputs[1].default_value[0],
                                            node.inputs[1].default_value[1],
                                            node.inputs[1].default_value[2])

        if node.inputs[2].is_linked:
            b_input = recursive_color_trace(mat_obj, mesh_obj, blend_path, node.inputs[2].links[0].from_node, node.inputs[2].links[0].from_socket)
        else:
            b_input = 'vec3(%f, %f, %f)' % (node.inputs[2].default_value[0],
                                            node.inputs[2].default_value[1],
                                            node.inputs[2].default_value[2])

        if node.blend_type == 'MULTIPLY':
            return '(%s * %s)' % (a_input, b_input)
        elif node.blend_type == 'ADD':
            return '(%s + %s)' % (a_input, b_input)
        else:
            raise RuntimeError("RMDL does not support shaders with '{0}' blending modes".format(node.blend_type))

    elif node.type == 'TEXTURE':

        if not node.inputs['Vector'].is_linked:
            raise RuntimeError("RMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")

        # Determine matrix generator type
        matrix_str = None
        soc_from = node.inputs['Vector'].links[0].from_socket

        if soc_from.node.type == 'GROUP':
            matrix_str = '%s(' % soc_from.node.node_tree.name
            for s in range(len(soc_from.node.inputs)-1):
                soc = soc_from.node.inputs[s+1]
                if len(soc.links):
                    raise RuntimeError("UV Modifier nodes may not have parameter links (default values only)")
                ncomps = len(soc.default_value)
                if ncomps > 1:
                    matrix_str += 'vec%d(' % ncomps
                    for c in ncomps-1:
                        matrix_str += '%f, ' % soc.default_value[c]
                    matrix_str += '%f)' % soc.default_value[ncomps-1]
                else:
                    matrix_str += '%f' % soc.default_value

                if s == len(soc_from.node.inputs)-2:
                    matrix_str += ')'
                else:
                    matrix_str += ', '

            soc_from = soc_from.node.inputs[0].links[0].from_socket

        elif soc_from.node.type == 'GEOMETRY':
            pass

        else:
            raise RuntimeError("RMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")

        if soc_from.node.type != 'GEOMETRY':
            raise RuntimeError("Matrix animator nodes must connect to 'Geometry' node")


        # Resolve map and matrix index
        node_label = soc_from.node.label
        if not matrix_str and node_label.startswith('MTX_'):
            matrix_str = 'hecl_TexMtx[%d]' % int(node_label[4:])

        if soc_from.name == 'UV':
            uv_name = soc_from.node.uv_layer
            uv_idx = mesh_obj.data.uv_layers.find(uv_name)
            uvsource_str = 'hecl_TexCoord[%d]' % uv_idx

        elif soc_from.name == 'Normal':
            uvsource_str = 'hecl_TexCoordModelViewNormal'

        elif soc_from.name == 'View':
            uvsource_str = 'hecl_TexCoordModelViewPosition'

        else:
            raise RuntimeError("Only the 'UV', 'Normal' and 'View' sockets may be used from 'Geometry' nodes")

        if socket.name == 'Value':
            if matrix_str:
                return 'texture("%s:%s", %s, %s).a' % (blend_path, node.texture.name, uvsource_str, matrix_str)
            else:
                return 'texture("%s:%s", %s).a' % (blend_path, node.texture.name, uvsource_str)
        if socket.name == 'Color':
            if matrix_str:
                return 'texture("%s:%s", %s, %s)' % (blend_path, node.texture.name, uvsource_str, matrix_str)
            else:
                return 'texture("%s:%s", %s)' % (blend_path, node.texture.name, uvsource_str)
        else:
            raise RuntimeError("Only the 'Value' or 'Color' output sockets may be used from Texture nodes")

    elif node.type == 'RGB':

        if node.label.startswith('DYNAMIC_'):
            dynamic_index = int(node.label[8:])
            return 'hecl_KColor[%d]' % dynamic_index

        return '%f' % node.outputs['Color'].default_value

    elif node.type == 'MATERIAL':
        
        if mat_obj.use_shadeless:
            return 'vec3(1.0)'
        else:
            return 'hecl_Lighting'

    else:
        raise RuntimeError("RMDL is unable to process '{0}' shader nodes in '{1}'".format(node.type, mat_obj.name))



# Trace alpha node structure
def recursive_alpha_trace(mat_obj, mesh_obj, blend_path, node, socket=None):

    if node.type == 'OUTPUT':
        if node.inputs['Alpha'].is_linked:
            return recursive_alpha_trace(mat_obj, mesh_obj, blend_path, node.inputs['Alpha'].links[0].from_node, node.inputs['Alpha'].links[0].from_socket)
        else:
            return '%f' % node.inputs['Alpha'].default_value
    
    elif node.type == 'MATH':
    
        if node.inputs[0].is_linked:
            a_input = recursive_alpha_trace(mat_obj, mesh_obj, blend_path, node.inputs[0].links[0].from_node, node.inputs[0].links[0].from_socket)
        else:
            a_input = '%f' % node.inputs[0].default_value

        if node.inputs[1].is_linked:
            b_input = recursive_alpha_trace(plat, mat_obj, mesh_obj, tex_list, mtx_dict, node.inputs[1].links[0].from_node, node.inputs[1].links[0].from_socket)
        else:
            b_input = '%f' % node.inputs[1].default_value
        
        if node.operation == 'MULTIPLY':
            return '(%s * %s)' % (a_input, b_input)
        elif node.operation == 'ADD':
            return '(%s + %s)' % (a_input, b_input)
        else:
            raise RuntimeError("RMDL does not support shaders with '{0}' blending modes".format(node.operation))

    elif node.type == 'TEXTURE':

        if not node.inputs['Vector'].is_linked:
            raise RuntimeError("RMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")
        
        # Determine matrix generator type
        matrix_str = None
        soc_from = node.inputs['Vector'].links[0].from_socket

        if soc_from.node.type == 'GROUP':
            matrix_str = '%s(' % soc_from.node.node_tree.name
            for s in range(len(soc_from.node.inputs)-1):
                soc = soc_from.node.inputs[s+1]
                if len(soc.links):
                    raise RuntimeError("UV Modifier nodes may not have parameter links (default values only)")
                ncomps = len(soc.default_value)
                if ncomps > 1:
                    matrix_str += 'vec%d(' % ncomps
                    for c in ncomps-1:
                        matrix_str += '%f, ' % soc.default_value[c]
                    matrix_str += '%f)' % soc.default_value[ncomps-1]
                else:
                    matrix_str += '%f' % soc.default_value

                if s == len(soc_from.node.inputs)-2:
                    matrix_str += ')'
                else:
                    matrix_str += ', '

            soc_from = soc_from.node.inputs[0].links[0].from_socket

        elif soc_from.node.type == 'GEOMETRY':
            pass

        else:
            raise RuntimeError("RMDL texture nodes must have a 'Geometry', 'Group' UV modifier node linked")

        if soc_from.node.type != 'GEOMETRY':
            raise RuntimeError("Matrix animator nodes must connect to 'Geometry' node")


        # Resolve map and matrix index
        node_label = soc_from.node.label
        if not matrix_str and node_label.startswith('MTX_'):
            matrix_str = 'hecl_TexMtx[%d]' % int(node_label[4:])

        if soc_from.name == 'UV':
            uv_name = soc_from.node.uv_layer
            uv_idx = mesh_obj.data.uv_layers.find(uv_name)
            uvsource_str = 'hecl_TexCoord[%d]' % uv_idx
            
        elif soc_from.name == 'Normal':
            uvsource_str = 'hecl_TexCoordModelViewNormal'

        elif soc_from.name == 'View':
            uvsource_str = 'hecl_TexCoordModelViewPosition'

        else:
            raise RuntimeError("Only the 'UV', 'Normal' and 'View' sockets may be used from 'Geometry' nodes")
        
        if socket.name == 'Value':
            if matrix_str:
                return 'texture("%s:%s", %s, %s).a' % (blend_path, node.texture.name, uvsource_str, matrix_str)
            else:
                return 'texture("%s:%s", %s).a' % (blend_path, node.texture.name, uvsource_str)
        else:
            raise RuntimeError("Only the 'Value' output sockets may be used from Texture nodes")

    elif node.type == 'VALUE':

        if node.label.startswith('DYNAMIC_'):
            dynamic_index = int(node.label[8:])
            return 'hecl_KColor[%d].a' % dynamic_index

        return '%f' % node.outputs['Value'].default_value

    elif node.type == 'MATERIAL':
    
        return '1.0'

    else:
        raise RuntimeError("RMDL is unable to process '{0}' shader nodes in '{1}'".format(node.type, mat_obj.name))



def shader(mat_obj, mesh_obj, blend_path):

    if not mat_obj.use_nodes:
        raise RuntimeError("RMDL *requires* that shader nodes are used; '{0}' does not".format(mat_obj.name))

    if 'Output' not in mat_obj.node_tree.nodes or mat_obj.node_tree.nodes['Output'].type != 'OUTPUT':
        raise RuntimeError("RMDL *requires* that an OUTPUT shader node named 'Output' is present")

    # Root (output) node
    output_node = mat_obj.node_tree.nodes['Output']

    # Trace nodes and build result
    color_trace_result = recursive_color_trace(mat_obj, mesh_obj, blend_path, output_node)
    alpha_trace_result = recursive_alpha_trace(mat_obj, mesh_obj, blend_path, output_node)

    blend_src = 'hecl_One'
    blend_dest = 'hecl_Zero'
    if mat_obj.game_settings.alpha_blend == 'ALPHA' or mat_obj.game_settings.alpha_blend == 'ALPHA_SORT':
        blend_src = 'hecl_SrcAlpha'
        blend_dest = 'hecl_OneMinusSrcAlpha'
    elif mat_obj.game_settings.alpha_blend == 'ADD':
        blend_src = 'hecl_SrcAlpha'
        blend_dest = 'hecl_One'

    # All done!
    return '''\
hecl_BlendSrcFactor = %s;
hecl_BlendDestFactor = %s;
hecl_FragColor[0] = %s;
hecl_FragColor[0].a = %s;
''' % (blend_src, blend_dest, color_trace_result, alpha_trace_result)

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
        shad = shader(context.object.active_material, context.object, bpy.data.filepath)
        
        vs = bpy.data.texts.new('HECL SHADER')
        vs.write(shad)

        return {'FINISHED'}
