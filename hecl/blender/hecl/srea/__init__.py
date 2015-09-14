import bpy
from .. import Nodegrid

# Trace color output searching for material node and making list from it
def recursive_build_material_chain(node):
    if node.type == 'OUTPUT':
        if node.inputs[0].is_linked:
            ret = recursive_build_material_chain(node.inputs[0].links[0].from_node)
            if ret:
                ret.append(node)
                return ret
    elif node.type == 'MIX_RGB':
        if node.inputs[1].is_linked:
            ret = recursive_build_material_chain(node.inputs[1].links[0].from_node)
            if ret:
                ret.append(node)
                return ret
        if node.inputs[2].is_linked:
            ret = recursive_build_material_chain(node.inputs[2].links[0].from_node)
            if ret:
                ret.append(node)
                return ret
    elif node.type == 'MATERIAL':
        return [node]
    return None

# Get Diffuse and Emissive output sockets from chain
def get_de_sockets(chain):
    found_add = None
    found_mul = None
    n = None
    for nn in reversed(chain):
        if not n:
            n = nn
            continue
        if n.type == 'MIX_RGB':
            if not found_add and n.blend_type == 'ADD' and nn.type != 'MATERIAL':
                use_idx = 1
                if n.inputs[1].is_linked:
                    tn = n.inputs[1].links[0].from_node
                    if tn == nn:
                        use_idx = 2
                if n.inputs[use_idx].is_linked:
                    found_add = n.inputs[use_idx].links[0].from_socket
                found_mul = None
            elif n.blend_type == 'MULTIPLY':
                use_idx = 1
                if n.inputs[1].is_linked:
                    tn = n.inputs[1].links[0].from_node
                    if tn == nn:
                        use_idx = 2
                if n.inputs[use_idx].is_linked:
                    found_mul = n.inputs[use_idx].links[0].from_socket
        n = nn

    return found_mul, found_add

# Get texture node from node
def tex_node_from_node(node):
    if node.type == 'TEXTURE':
        return node
    elif node.type == 'MIX_RGB':
        if node.inputs[1].is_linked:
            ret = image_from_node(node.inputs[1].links[0].from_node)
            if ret:
                return ret
        if node.inputs[2].is_linked:
            ret = image_from_node(node.inputs[2].links[0].from_node)
            if ret:
                return ret
    return None

# Delete existing cycles nodes and convert from GLSL nodes
CYCLES_TYPES = {'OUTPUT_MATERIAL', 'ADD_SHADER', 'BSDF_DIFFUSE', 'BSDF_TRANSPARENT', 'EMISSION', 'TEX_IMAGE'}
def configure_nodetree_cycles(mat, pixel_size):
    nt = mat.node_tree
    to_remove = set()
    for node in nt.nodes:
        if node.type in CYCLES_TYPES:
            to_remove.add(node)
            if node.parent:
                to_remove.add(node.parent)
    for node in to_remove:
        nt.nodes.remove(node)

    gridder = Nodegrid.Nodegrid(nt, cycles=True)

    image_out_node = None
    if mat.retro_lightmap != '':

        # Get name of lightmap texture
        if mat.retro_lightmap in bpy.data.textures:
            img_name = mat.retro_lightmap
            if img_name in bpy.data.images:
                bpy.data.textures[mat.retro_lightmap].image = bpy.data.images[img_name]
            else:
                bpy.data.textures[mat.retro_lightmap].image = None


        # Determine if image already established
        new_image = None
        tex_name = mat.retro_lightmap + '_CYCLES'
        if tex_name in bpy.data.images:
            new_image = bpy.data.images[tex_name]
        else:

            # New image; determine proportional aspect
            old_image = bpy.data.images[mat.retro_lightmap]
            width_fac = 1
            height_fac = 1
            if old_image.size[0] > old_image.size[1]:
                height_fac = old_image.size[0] // old_image.size[1]
            else:
                width_fac = old_image.size[1] // old_image.size[0]

            # Make image and establish in filesystem
            new_image = bpy.data.images.new(tex_name, pixel_size // width_fac, pixel_size // height_fac)
            new_image.use_fake_user = True
            new_image.file_format = 'PNG'
            new_image.filepath = old_image.filepath[:len(old_image.filepath)-4] + '_CYCLES.png'

        image_out_node = nt.nodes.new('ShaderNodeTexImage')
        image_out_node.name = 'CYCLES_OUT'
        gridder.place_node(image_out_node, 3)
        image_out_node.image = new_image


    if mat.game_settings.alpha_blend == 'ADD':
        transp = nt.nodes.new('ShaderNodeBsdfTransparent')
        gridder.place_node(transp, 2)
        material_output = nt.nodes.new('ShaderNodeOutputMaterial')
        gridder.place_node(material_output, 3)
        nt.links.new(transp.outputs[0], material_output.inputs[0])

    elif mat.game_settings.alpha_blend == 'ALPHA':
        transp = nt.nodes.new('ShaderNodeBsdfTransparent')
        gridder.place_node(transp, 2)
        material_output = nt.nodes.new('ShaderNodeOutputMaterial')
        gridder.place_node(material_output, 3)
        nt.links.new(transp.outputs[0], material_output.inputs[0])

        # Classify connected transparent textures
        chain = recursive_build_material_chain(nt.nodes['Output'])
        if chain:
            diffuse_soc, emissive_soc = get_de_sockets(chain)
            tex_node = tex_node_from_node(diffuse_soc.node)
            diffuse_image_node = nt.nodes.new('ShaderNodeTexImage')
            gridder.place_node(diffuse_image_node, 1)
            diffuse_image_node.image = tex_node.texture.image
            mapping = nt.nodes.new('ShaderNodeMapping')
            gridder.place_node(mapping, 2)
            mapping.vector_type = 'TEXTURE'
            mapping.translation = (1.0,1.0,0.0)
            mapping.scale = (2.0,2.0,1.0)
            nt.links.new(diffuse_image_node.outputs[0], transp.inputs[0])
            nt.links.new(tex_node.inputs[0].links[0].from_socket, mapping.inputs[0])
            nt.links.new(mapping.outputs[0], diffuse_image_node.inputs[0])

    else:
        # Classify connected opaque textures
        chain = recursive_build_material_chain(nt.nodes['Output'])
        if chain:
            diffuse_soc, emissive_soc = get_de_sockets(chain)
            emissive_soc = None
            if diffuse_soc:
                tex_node = tex_node_from_node(diffuse_soc.node)
                diffuse_image_node = nt.nodes.new('ShaderNodeTexImage')
                gridder.place_node(diffuse_image_node, 1)
                diffuse_image_node.image = tex_node.texture.image
                mapping = nt.nodes.new('ShaderNodeMapping')
                gridder.place_node(mapping, 2)
                mapping.vector_type = 'TEXTURE'
                mapping.translation = (1.0,1.0,0.0)
                mapping.scale = (2.0,2.0,1.0)
                diffuse = nt.nodes.new('ShaderNodeBsdfDiffuse')
                gridder.place_node(diffuse, 2)
                nt.links.new(diffuse_image_node.outputs[0], diffuse.inputs[0])
                nt.links.new(tex_node.inputs[0].links[0].from_socket, mapping.inputs[0])
                nt.links.new(mapping.outputs[0], diffuse_image_node.inputs[0])
            if emissive_soc:
                emissive = nt.nodes.new('ShaderNodeEmission')
                gridder.place_node(emissive, 2)
                nt.links.new(emissive_soc, emissive.inputs[0])

            material_output = nt.nodes.new('ShaderNodeOutputMaterial')
            gridder.place_node(material_output, 3)
            if diffuse_soc and emissive_soc:
                shader_add = nt.nodes.new('ShaderNodeAddShader')
                gridder.place_node(shader_add, 2)
                nt.links.new(diffuse.outputs[0], shader_add.inputs[0])
                nt.links.new(emissive.outputs[0], shader_add.inputs[1])
                nt.links.new(shader_add.outputs[0], material_output.inputs[0])
            elif diffuse_soc:
                nt.links.new(diffuse.outputs[0], material_output.inputs[0])
            elif emissive_soc:
                nt.links.new(emissive.outputs[0], material_output.inputs[0])

    # Set bake target node active
    if image_out_node:
        nt.nodes.active = image_out_node
    else:
        image_out_node = nt.nodes.new('ShaderNodeTexImage')
        nt.nodes.active = image_out_node
        image_out_node.name = 'CYCLES_OUT'
        gridder.place_node(image_out_node, 3)
        if 'FAKE' in bpy.data.images:
            image_out_node.image = bpy.data.images['FAKE']
        else:
            fake_img = bpy.data.images.new('FAKE', 1, 1)
            image_out_node.image = fake_img

# Lightmap render operator
class SREAConfigureCycles(bpy.types.Operator):
    bl_idname = "scene.hecl_area_configure_cycles"
    bl_label = "HECL Configure Cycles"
    bl_description = "Setup Cycles nodes for lightmap baking"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and context.scene.hecl_type == 'AREA')

    def execute(self, context):
        area_data = context.scene.hecl_srea_data

        # Resolution
        pixel_size = int(area_data.lightmap_resolution)

        # Iterate materials and setup cycles
        for mat in bpy.data.materials:
            if mat.use_nodes:
                configure_nodetree_cycles(mat, pixel_size)
        return {'FINISHED'}


# Area data class
class SREAData(bpy.types.PropertyGroup):
    lightmap_resolution = bpy.props.EnumProperty(name="HECL Area Lightmap Resolution",
            description="Set square resolution to use when rendering new lightmaps",
            items=[
            ('256', "256", "256x256 (original quality)"),
            ('512', "512", "512x512"),
            ('1024', "1024", "1024x1024"),
            ('2048', "2048", "2048x2048"),
            ('4096', "4096", "4096x4096")],
            default='1024')



# Lightmap render operator
class SREARenderLightmaps(bpy.types.Operator):
    bl_idname = "scene.hecl_area_render_lightmaps"
    bl_label = "HECL Render New Lightmaps"
    bl_description = "Bake new lightmaps for HECL runtime"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and context.scene.hecl_type == 'AREA')

    def execute(self, context):
        area_data = context.scene.hecl_srea_data
        if (context.scene is not None and context.scene.hecl_type == 'AREA'):

            # Mmm Cycles
            context.scene.render.engine = 'CYCLES'
            context.scene.cycles.use_clear = False
            context.scene.cycles.bake_type = 'COMBINED'

            # Resolution
            pixel_size = int(area_data.lightmap_resolution)

            # Iterate materials and setup cycles
            for mat in bpy.data.materials:
                if mat.use_nodes:
                    configure_nodetree_cycles(mat, pixel_size)

            # Bake operation runs with object selection
            bpy.ops.object.select_all(action='DESELECT')

            # Iterate mesh objects and establish images for receiving baked renders
            for obj in context.scene.objects:
                if obj.type == 'MESH':

                    if not len(obj.data.uv_textures):
                        continue

                    # Make correct UV layer active
                    obj.data.uv_textures.active_index = 0

                    bpy.context.scene.objects.active = obj
                    obj.select = True


            # Make lightmaps
            bpy.ops.object.bake()

            # Save images to disk
            for img in bpy.data.images:
                if img.name.endswith('_CYCLES'):
                    img.save()

        return {'FINISHED'}

# Cook
def cook(writebuffunc, platform, endianchar):
    print('COOKING SREA')

# Panel draw
def draw(layout, context):
    pass

# Registration
def register():
    bpy.utils.register_class(SREAData)
    bpy.utils.register_class(SREAConfigureCycles)
    bpy.utils.register_class(SREARenderLightmaps)
    bpy.types.Scene.hecl_srea_data = bpy.props.PointerProperty(type=SREAData)
    bpy.types.Material.retro_lightmap = bpy.props.StringProperty(name='Retro: Lightmap')

def unregister():
    bpy.utils.unregister_class(SREAData)
    bpy.utils.unregister_class(SREAConfigureCycles)
    bpy.utils.unregister_class(SREARenderLightmaps)

