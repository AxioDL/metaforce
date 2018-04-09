import bpy
from bpy.app.handlers import persistent
from mathutils import Quaternion, Color
import math
import os.path
from .. import Nodegrid, swld

# Preview update func (for lighting preview)
def preview_update(self, context):
    if context.scene.hecl_type == 'AREA':
        area_data = context.scene.hecl_srea_data

        # Original Lightmaps
        if area_data.lightmap_mode == 'ORIGINAL':
            for material in bpy.data.materials:
                if material.hecl_lightmap:
                    material.use_shadeless = False

                    # Reference original game lightmaps
                    if material.hecl_lightmap in bpy.data.textures:
                        img_name = material.hecl_lightmap
                        if img_name in bpy.data.images:
                            bpy.data.textures[material.hecl_lightmap].image = bpy.data.images[img_name]
                        else:
                            bpy.data.textures[material.hecl_lightmap].image = None

        # Cycles Lightmaps
        elif area_data.lightmap_mode == 'CYCLES':
            for material in bpy.data.materials:
                if material.hecl_lightmap:
                    material.use_shadeless = False

                    # Reference newly-generated lightmaps
                    if material.hecl_lightmap in bpy.data.textures:
                        img_name = material.hecl_lightmap + '_CYCLES'
                        if img_name in bpy.data.images:
                            bpy.data.textures[material.hecl_lightmap].image = bpy.data.images[img_name]
                        else:
                            bpy.data.textures[material.hecl_lightmap].image = None
        # White Lightmaps
        elif area_data.lightmap_mode == 'NONE':
            img_name = 'NONE'
            img = None
            if img_name in bpy.data.images:
                img = bpy.data.images[img_name]
            else:
                img = bpy.data.images.new(img_name, width=1, height=1)
                pixels = [1.0] * (4 * 1 * 1)
                img.pixels = pixels
                img.use_fake_user = True
                img.file_format = 'PNG'

            for material in bpy.data.materials:
                if material.hecl_lightmap:
                    material.use_shadeless = False

                    # Reference NONE
                    if material.hecl_lightmap in bpy.data.textures:
                        bpy.data.textures[material.hecl_lightmap].image = img
                            

# Update lightmap output-resolution
def set_lightmap_resolution(self, context):
    area_data = context.scene.hecl_srea_data
    pixel_size = int(area_data.lightmap_resolution)
    for mat in bpy.data.materials:
        if not mat.hecl_lightmap:
            continue

        # Determine proportional aspect
        old_image = bpy.data.images[mat.hecl_lightmap]
        width_fac = 1
        height_fac = 1
        if old_image.size[0] > old_image.size[1]:
            height_fac = old_image.size[0] // old_image.size[1]
        else:
            width_fac = old_image.size[1] // old_image.size[0]
        width = pixel_size // width_fac
        height = pixel_size // height_fac

        # Find target texture and scale connected image
        if 'CYCLES_OUT' in mat.node_tree.nodes:
            out_node = mat.node_tree.nodes['CYCLES_OUT']
            if out_node.type == 'TEX_IMAGE':
                image = out_node.image
                if image:
                    image.scale(width, height)

def make_or_load_cycles_image(mat, area_data):
    if not mat.hecl_lightmap:
        return
    pixel_size = int(area_data.lightmap_resolution)
    tex_name = mat.hecl_lightmap + '_CYCLES'
    if area_data.adjacent_area < 0:
        path_name = '//' + mat.hecl_lightmap + '_CYCLES.png'
    else:
        path_name = '//' + mat.hecl_lightmap + '_CYCLES_%d.png' % area_data.adjacent_area

    # Determine proportional aspect
    old_image = bpy.data.images[mat.hecl_lightmap]
    width_fac = 1
    height_fac = 1
    if old_image.size[0] > old_image.size[1]:
        height_fac = old_image.size[0] // old_image.size[1]
    else:
        width_fac = old_image.size[1] // old_image.size[0]
    width = pixel_size // width_fac
    height = pixel_size // height_fac

    # Check for consistency with on-disk image
    if tex_name in bpy.data.images:
        image = bpy.data.images[tex_name]
        image.use_fake_user = True
        image.file_format = 'PNG'
        image.filepath = path_name
        good = True
        if image.size[0] != width or image.size[1] != height:
            try:
                image.scale(width, height)
            except:
                good = False
        if good:
            return image
        # Remove and recreate if we get here
        bpy.data.images.remove(bpy.data.images[tex_name])

    # New image (or load from disk if available)
    try:
        new_image = bpy.data.images.load(path_name)
        new_image.name = tex_name
        new_image.use_fake_user = True
        if new_image.size[0] != width or new_image.size[1] != height:
            new_image.scale(width, height)
    except:
        new_image = bpy.data.images.new(tex_name, width, height)
        new_image.use_fake_user = True
        new_image.file_format = 'PNG'
        new_image.filepath = path_name

    return new_image

# Set adjacent area lightmaps
def set_adjacent_area(self, context):
    bg_scene = context.scene.background_set
    dock_idx = context.scene.hecl_srea_data.adjacent_area
    if bg_scene is None:
        self.report({'ERROR_INVALID_CONTEXT'}, 'No background world scene is set')
        return

    if bg_scene.hecl_type != 'WORLD':
        self.report({'ERROR_INVALID_CONTEXT'}, 'Scene "%s" is not a hecl WORLD' % bg_scene.name)
        return

    adjacent = dock_idx >= 0
    if len(context.scene.render.layers):
        context.scene.render.layers[0].use_sky = not adjacent

    # Remove linked lamps and show/hide locals
    for obj in bpy.data.objects:
        if obj.library is not None and (obj.type == 'LAMP' or obj.type == 'MESH'):
            try:
                context.scene.objects.unlink(obj)
            except:
                pass
            continue
        if obj.type == 'LAMP':
            obj.hide_render = adjacent

    # Remove linked scenes
    to_remove = []
    for scene in bpy.data.scenes:
        if scene.hecl_type == 'AREA' and scene.library is not None:
            to_remove.append(scene)
    for scene in to_remove:
        bpy.data.scenes.remove(scene)

    # Link scene, meshes, and lamps
    if dock_idx >= 0:
        other_area_name = get_other_area_name(self, bg_scene, dock_idx)
        if other_area_name is None:
            return
        other_area_scene_name = None
        this_dir = os.path.split(bpy.data.filepath)[0]
        try:
            with bpy.data.libraries.load('%s/../%s/!area.blend' % (this_dir, other_area_name),
                                         link=True, relative=True) as (data_from, data_to):
                for scene in data_from.scenes:
                    other_area_scene_name = scene
                    data_to.scenes = [other_area_scene_name]
                    break
        except Exception as e:
            self.report({'ERROR_INVALID_CONTEXT'}, 'Unable to open "%s" blend file: %s' % (other_area_name, str(e)))
            return
        if other_area_scene_name is None:
            self.report({'ERROR_INVALID_CONTEXT'}, '"%s" does not have an area scene' % other_area_name)
            return
        other_scene = bpy.data.scenes[other_area_scene_name]
        if other_scene.hecl_type != 'AREA':
            self.report({'ERROR_INVALID_CONTEXT'}, '"%s" does not have an area scene' % other_area_name)
            bpy.data.scenes.remove(other_scene)
            return
        for obj in other_scene.objects:
            if (obj.type == 'LAMP' or obj.type == 'MESH') and obj.layers[0]:
                context.scene.objects.link(obj)
                obj.hide_render = False

    # Ensure filepaths target the current dock index
    for mat in bpy.data.materials:
        if not mat.library and mat.use_nodes and 'CYCLES_OUT' in mat.node_tree.nodes:
            texture_node = mat.node_tree.nodes['CYCLES_OUT']
            texture_node.image = make_or_load_cycles_image(mat, context.scene.hecl_srea_data)

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
            update=set_lightmap_resolution,
            default='1024')

    lightmap_mode = bpy.props.EnumProperty(name="HECL Area Lightmap Mode",
            description="Simple way to manipulate all lightmap-using materials",
            items=[
            ('NONE', "None", "Pure white lightmaps"),
            ('ORIGINAL', "Original", "Original extracted lightmaps"),
            ('CYCLES', "Cycles", "Blender-rendered lightmaps")],
            update=preview_update,
            default='ORIGINAL')

    adjacent_area = bpy.props.IntProperty(name="HECL Adjacent Area Lightmap",
            description="Dock index of adjacent area to render, or -1 for local lights",
            update=set_adjacent_area,
            default=-1,
            min=-1,
            max=8)

    def report(self, code, string):
        pass

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
CYCLES_TYPES = {'OUTPUT_MATERIAL', 'ADD_SHADER', 'BSDF_DIFFUSE', 'BSDF_TRANSPARENT',
                'EMISSION', 'MIX_SHADER', 'TEX_IMAGE'}
def initialize_nodetree_cycles(mat, area_data):
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

    if mat.hecl_lightmap and not mat.library:
        # Get name of lightmap texture
        if mat.hecl_lightmap in bpy.data.textures:
            img_name = mat.hecl_lightmap
            if img_name in bpy.data.images:
                bpy.data.textures[mat.hecl_lightmap].image = bpy.data.images[img_name]
            else:
                bpy.data.textures[mat.hecl_lightmap].image = None

        # Get image already established or make new one
        new_image = make_or_load_cycles_image(mat, area_data)

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
        diffuse = nt.nodes.new('ShaderNodeBsdfDiffuse')
        gridder.place_node(diffuse, 2)
        transp = nt.nodes.new('ShaderNodeBsdfTransparent')
        gridder.place_node(transp, 2)
        mix_shader = nt.nodes.new('ShaderNodeMixShader')
        gridder.place_node(mix_shader, 2)
        nt.links.new(transp.outputs[0], mix_shader.inputs[1])
        nt.links.new(diffuse.outputs[0], mix_shader.inputs[2])
        material_output = nt.nodes.new('ShaderNodeOutputMaterial')
        gridder.place_node(material_output, 3)
        nt.links.new(mix_shader.outputs[0], material_output.inputs[0])

        # Classify connected transparent textures
        chain = recursive_build_material_chain(nt.nodes['Output'])
        if chain:
            diffuse_soc, emissive_soc = get_de_sockets(chain)
            tex_node = tex_node_from_node(diffuse_soc.node)
            if tex_node and tex_node.inputs[0].links[0].from_socket.name == 'UV':
                diffuse_image_node = nt.nodes.new('ShaderNodeTexImage')
                gridder.place_node(diffuse_image_node, 1)
                diffuse_image_node.image = tex_node.texture.image
                mixrgb_node = nt.nodes.new('ShaderNodeMixRGB')
                gridder.place_node(mixrgb_node, 1)
                mixrgb_node.inputs[1].default_value = (1.0,1.0,1.0,1.0)
                mapping = nt.nodes.new('ShaderNodeUVMap')
                gridder.place_node(mapping, 1)
                mapping.uv_map = tex_node.inputs[0].links[0].from_node.uv_layer
                nt.links.new(diffuse_image_node.outputs[0], diffuse.inputs[0])
                nt.links.new(diffuse_image_node.outputs[0], mixrgb_node.inputs[2])
                if nt.nodes['Output'].inputs[1].is_linked:
                    nt.links.new(nt.nodes['Output'].inputs[1].links[0].from_socket, mix_shader.inputs[0])
                    nt.links.new(nt.nodes['Output'].inputs[1].links[0].from_socket, mixrgb_node.inputs[0])
                nt.links.new(mixrgb_node.outputs[0], transp.inputs[0])
                nt.links.new(mapping.outputs[0], diffuse_image_node.inputs[0])

    else:
        # Classify connected opaque textures
        chain = recursive_build_material_chain(nt.nodes['Output'])
        if chain:
            diffuse_soc, emissive_soc = get_de_sockets(chain)
            emissive_soc = None
            if diffuse_soc:
                tex_node = tex_node_from_node(diffuse_soc.node)
                if tex_node and tex_node.inputs[0].links[0].from_socket.name == 'UV':
                    diffuse_image_node = nt.nodes.new('ShaderNodeTexImage')
                    gridder.place_node(diffuse_image_node, 1)
                    diffuse_image_node.image = tex_node.texture.image
                    mapping = nt.nodes.new('ShaderNodeUVMap')
                    gridder.place_node(mapping, 1)
                    mapping.uv_map = tex_node.inputs[0].links[0].from_node.uv_layer
                    diffuse = nt.nodes.new('ShaderNodeBsdfDiffuse')
                    gridder.place_node(diffuse, 2)
                    nt.links.new(diffuse_image_node.outputs[0], diffuse.inputs[0])
                    nt.links.new(mapping.outputs[0], diffuse_image_node.inputs[0])
                else:
                    diffuse = nt.nodes.new('ShaderNodeBsdfDiffuse')
                    gridder.place_node(diffuse, 2)
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

# Lightmap setup operator
class SREAInitializeCycles(bpy.types.Operator):
    bl_idname = "scene.hecl_area_initialize_cycles"
    bl_label = "HECL Initialize Cycles"
    bl_description = "Initialize Cycles nodes for lightmap baking (WILL DELETE EXISTING CYCLES NODES!)"

    @classmethod
    def poll(cls, context):
        return context.scene is not None and context.scene.hecl_type == 'AREA'

    def execute(self, context):
        area_data = context.scene.hecl_srea_data

        # Iterate materials and setup cycles
        for mat in bpy.data.materials:
            if mat.use_nodes:
                initialize_nodetree_cycles(mat, area_data)
        return {'FINISHED'}

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

# Lookup the directory name of other area via dock link
def get_other_area_name(op, bg_scene, dock_idx):
    dock_conns = swld.build_dock_connections(bg_scene)
    this_dir = os.path.split(bpy.data.filepath)[0]
    this_area_name = os.path.basename(this_dir)
    wld_area = next((area for area in dock_conns[0] if area[0].name == this_area_name), None)
    if wld_area is None:
        op.report({'ERROR_INVALID_CONTEXT'}, 'Unable to resolve area in world')
        return None
    if dock_idx not in range(len(wld_area[1])):
        op.report({'ERROR_INVALID_CONTEXT'}, 'Dock %d is out of this area\'s range [0,%d]' %
                  (dock_idx, len(wld_area[1])))
        return None
    dock_obj = wld_area[1][dock_idx]
    if dock_obj.name not in dock_conns[1]:
        op.report({'ERROR_INVALID_CONTEXT'}, 'Unable to find sister dock for %s' % dock_obj.name)
        return None
    other_wld_area = dock_conns[1][dock_obj.name][2].parent
    if other_wld_area is None:
        op.report({'ERROR_INVALID_CONTEXT'}, '%s does not have a parent area' % dock_obj.name)
        return None
    return other_wld_area.name

# Shared lightmap render procedure
def render_lightmaps(context):
    if context.scene is not None:
        area_data = context.scene.hecl_srea_data

        # Resolution
        pixel_size = int(area_data.lightmap_resolution)

        # Mmm Cycles
        context.scene.render.engine = 'CYCLES'
        context.scene.render.bake.margin = pixel_size // 256

        # Iterate materials and setup cycles
        for mat in bpy.data.materials:
            if mat.use_nodes:
                # Set bake target node active
                if 'CYCLES_OUT' in mat.node_tree.nodes:
                    mat.node_tree.nodes.active = mat.node_tree.nodes['CYCLES_OUT']
                else:
                    image_out_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
                    mat.node_tree.nodes.active = image_out_node
                    image_out_node.name = 'CYCLES_OUT'
                    if 'FAKE' in bpy.data.images:
                        image_out_node.image = bpy.data.images['FAKE']
                    else:
                        fake_img = bpy.data.images.new('FAKE', 1, 1)
                        image_out_node.image = fake_img

        # Iterate mesh objects and set UV 0 as the active UV layer
        for obj in context.scene.objects:
            if obj.type == 'MESH':

                if not len(obj.data.uv_textures):
                    continue

                # Make correct UV layer active
                obj.data.uv_textures.active_index = 0

        # Make lightmaps
        bpy.ops.object.bake('INVOKE_DEFAULT', type='DIFFUSE', pass_filter={'DIRECT', 'INDIRECT'})

# Lightmap render operator
class SREARenderLightmaps(bpy.types.Operator):
    bl_idname = "scene.hecl_area_render_lightmaps"
    bl_label = "HECL Render New Lightmaps"
    bl_description = "Bake new lightmaps for HECL runtime"

    @classmethod
    def poll(cls, context):
        return context.scene is not None

    def execute(self, context):
        if not context.selected_objects:
            for obj in context.scene.objects:
                if obj.type == 'MESH' and not obj.library:
                    obj.select = True
                    context.scene.objects.active = obj

        render_lightmaps(context)

        return {'FINISHED'}

def shadeless_material(idx):
    name = 'SHADELESS_MAT_%d' % idx
    if name in bpy.data.materials:
        return bpy.data.materials[name]
    mat = bpy.data.materials.new(name)
    mat.use_shadeless = True
    r = idx % 256
    g = (idx % 65536) // 256
    b = idx // 65536
    mat.diffuse_color = Color((r / 255.0, g / 255.0, b / 255.0))
    return mat

look_forward = Quaternion((1.0, 0.0, 0.0), math.radians(90.0))
look_backward = Quaternion((0.0, 0.0, 1.0), math.radians(180.0)) * Quaternion((1.0, 0.0, 0.0), math.radians(90.0))
look_up = Quaternion((1.0, 0.0, 0.0), math.radians(180.0))
look_down = Quaternion((1.0, 0.0, 0.0), math.radians(0.0))
look_left = Quaternion((0.0, 0.0, 1.0), math.radians(90.0)) * Quaternion((1.0, 0.0, 0.0), math.radians(90.0))
look_right = Quaternion((0.0, 0.0, 1.0), math.radians(-90.0)) * Quaternion((1.0, 0.0, 0.0), math.radians(90.0))
look_list = (look_forward, look_backward, look_up, look_down, look_left, look_right)

# Render PVS for location
def render_pvs(pathOut, location):
    bpy.context.scene.render.resolution_x = 256
    bpy.context.scene.render.resolution_y = 256
    bpy.context.scene.render.resolution_percentage = 100
    bpy.context.scene.render.use_antialiasing = False
    bpy.context.scene.render.use_textures = False
    bpy.context.scene.render.use_shadows = False
    bpy.context.scene.render.use_sss = False
    bpy.context.scene.render.use_envmaps = False
    bpy.context.scene.render.use_raytrace = False
    bpy.context.scene.render.engine = 'BLENDER_RENDER'
    bpy.context.scene.display_settings.display_device = 'None'
    bpy.context.scene.render.image_settings.file_format = 'PNG'
    bpy.context.scene.world.horizon_color = Color((1.0, 1.0, 1.0))
    bpy.context.scene.world.zenith_color = Color((1.0, 1.0, 1.0))

    cam = bpy.data.cameras.new('CUBIC_CAM')
    cam_obj = bpy.data.objects.new('CUBIC_CAM', cam)
    bpy.context.scene.objects.link(cam_obj)
    bpy.context.scene.camera = cam_obj
    cam.lens_unit = 'FOV'
    cam.angle = math.radians(90.0)

    mat_idx = 0
    for obj in bpy.context.scene.objects:
        if obj.type == 'MESH':
            if obj.name == 'CMESH':
                continue
            mat = shadeless_material(mat_idx)
            for slot in obj.material_slots:
                slot.material = mat
            mat_idx += 1

    cam_obj.location = location
    cam_obj.rotation_mode = 'QUATERNION'

    for i in range(6):
        cam_obj.rotation_quaternion = look_list[i]
        bpy.context.scene.render.filepath = '%s%d' % (pathOut, i)
        bpy.ops.render.render(write_still=True)

    bpy.context.scene.camera = None
    bpy.context.scene.objects.unlink(cam_obj)
    bpy.data.objects.remove(cam_obj)
    bpy.data.cameras.remove(cam)

# Render PVS for light
def render_pvs_light(pathOut, lightName):
    if lightName not in bpy.context.scene.objects:
        raise RuntimeError('Unable to find light %s' % lightName)
    render_pvs(pathOut, bpy.context.scene.objects[lightName].location)

# Cook
def cook(writebuffunc, platform, endianchar):
    print('COOKING SREA')

# Panel draw
def draw(layout, context):
    area_data = context.scene.hecl_srea_data
    layout.label("Lighting:", icon='LAMP_SPOT')
    light_row = layout.row(align=True)
    light_row.prop_enum(area_data, 'lightmap_mode', 'NONE')
    light_row.prop_enum(area_data, 'lightmap_mode', 'ORIGINAL')
    light_row.prop_enum(area_data, 'lightmap_mode', 'CYCLES')
    layout.prop(area_data, 'lightmap_resolution', text="Resolution")
    layout.menu("CYCLES_MT_sampling_presets", text=bpy.types.CYCLES_MT_sampling_presets.bl_label)
    layout.prop(context.scene.render.bake, "use_clear", text="Clear Before Baking")
    layout.prop(area_data, 'adjacent_area', text='Adjacent Dock Index', icon='OOPS')
    layout.operator("scene.hecl_area_initialize_cycles", text="Initialize Cycles Nodes", icon='NODETREE')
    layout.operator("scene.hecl_area_render_lightmaps", text="Bake Cycles Lightmaps", icon='RENDER_STILL')
    layout.operator("image.save_dirty", text="Save Lightmaps", icon='FILE_TICK')


# Load scene callback
@persistent
def scene_loaded(dummy):
    preview_update(None, bpy.context)

# Registration
def register():
    bpy.utils.register_class(SREAData)
    bpy.utils.register_class(SREAInitializeCycles)
    bpy.utils.register_class(SREARenderLightmaps)
    bpy.types.Scene.hecl_srea_data = bpy.props.PointerProperty(type=SREAData)
    bpy.types.Material.hecl_lightmap = bpy.props.StringProperty(name='HECL: Lightmap Base Name')
    bpy.app.handlers.load_post.append(scene_loaded)

def unregister():
    bpy.utils.unregister_class(SREAData)
    bpy.utils.unregister_class(SREAInitializeCycles)
    bpy.utils.unregister_class(SREARenderLightmaps)

