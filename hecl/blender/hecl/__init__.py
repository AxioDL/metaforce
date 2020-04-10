bl_info = {
    "name": "HECL",
    "author": "Jack Andersen <jackoalan@gmail.com>",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "tracker_url": "https://github.com/AxioDL/hecl/issues/new",
    "location": "Properties > Scene > HECL",
    "description": "Enables blender to gather meshes, materials, and textures for hecl",
    "category": "System"}

# Package import
from . import hmdl, sact, srea, swld, armature, mapa, mapu, frme, path, Nodegrid, Patching
Nodegrid = Nodegrid.Nodegrid
parent_armature = sact.SACTSubtype.parent_armature
import bpy, os, sys, struct, math
from mathutils import Vector

# Appendable list allowing external addons to register additional resource types
hecl_typeS = [
('NONE', "None", "Active scene not using HECL", None),
('MESH', "Mesh", "Active scene represents an HMDL Mesh", hmdl.draw),
('CMESH', "Collision Mesh", "Active scene represents a Collision Mesh", None),
('ARMATURE', "Armature", "Active scene represents an Armature", armature.draw),
('ACTOR', "Actor", "Active scene represents a HECL Actor", sact.draw),
('AREA', "Area", "Active scene represents a HECL Area", srea.draw),
('WORLD', "World", "Active scene represents a HECL World", swld.draw),
('MAPAREA', "Map Area", "Active scene represents a HECL Map Area", mapa.draw),
('MAPUNIVERSE', "Map Universe", "Active scene represents a HECL Map Universe", mapu.draw),
('FRAME', "Gui Frame", "Active scene represents a HECL Gui Frame", frme.draw),
('PATH', "Path Mesh", "Active scene represents a HECL Path Mesh", path.draw)]

# Main Scene Panel
class hecl_scene_panel(bpy.types.Panel):
    bl_idname = "SCENE_PT_hecl"
    bl_label = "HECL"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None)

    def draw(self, context):
        layout = self.layout
        type_row = layout.row(align=True)
        type_row.prop_menu_enum(context.scene, 'hecl_type', text='Export Type')

        if context.scene.hecl_type == 'MESH' or context.scene.hecl_type == 'AREA' or context.scene.hecl_type == 'ACTOR':
            sm_row = layout.row(align=True)
            sm_row.prop_enum(context.scene, 'hecl_shader_model', 'ORIGINAL')
            sm_row.prop_enum(context.scene, 'hecl_shader_model', 'PBR')
            layout.prop(context.scene, 'hecl_mp3_bloom', text='View MP3 Bloom')

        for exp_type in hecl_typeS:
            if exp_type[0] == context.scene.hecl_type and callable(exp_type[3]):
                exp_type[3](self.layout, context)
                break

# Light Panel
class hecl_light_panel(bpy.types.Panel):
    bl_idname = "DATA_PT_hecl_light"
    bl_label = "HECL"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        return context.light

    def draw(self, context):
        layout = self.layout
        layout.prop(context.light, 'hecl_falloff_constant')
        layout.prop(context.light, 'hecl_falloff_linear')
        layout.prop(context.light, 'hecl_falloff_quadratic')

# Blender export-type registration
def register_export_type_enum():
    bpy.types.Scene.hecl_type = bpy.props.EnumProperty(items=
        [tp[:3] for tp in hecl_typeS],
        name="HECL Export Type",
        description="Selects how active scene is exported by HECL")

# Function for external addons to register export types with HECL
def add_export_type(type_tuple):
    type_tup = tuple(type_tuple)
    for tp in hecl_typeS:
        if tp[0] == type_tup[0]:
            raise RuntimeError("Type already registered with HECL")
    hecl_types.append(type_tup)
    register_export_type_enum()

# Shell command receiver (from HECL driver)
def command(cmdline, writepipeline, writepipebuf):
    pass

def mesh_aabb(writepipebuf):
    scene = bpy.context.scene
    total_min = Vector((99999.0, 99999.0, 99999.0))
    total_max = Vector((-99999.0, -99999.0, -99999.0))

    if bpy.context.scene.hecl_type == 'ACTOR':
        sact_data = bpy.context.scene.hecl_sact_data
        for subtype in sact_data.subtypes:
            if subtype.linked_mesh in bpy.data.objects:
                mesh = bpy.data.objects[subtype.linked_mesh]
                minPt = mesh.bound_box[0]
                maxPt = mesh.bound_box[6]
                for comp in range(3):
                    if minPt[comp] < total_min[comp]:
                        total_min[comp] = minPt[comp]
                for comp in range(3):
                    if maxPt[comp] > total_max[comp]:
                        total_max[comp] = maxPt[comp]

    elif bpy.context.scene.hecl_type == 'MESH':
        meshName = bpy.context.scene.hecl_mesh_obj
        if meshName in bpy.data.objects:
            mesh = bpy.data.objects[meshName]
            minPt = mesh.bound_box[0]
            maxPt = mesh.bound_box[6]
            for comp in range(3):
                if minPt[comp] < total_min[comp]:
                    total_min[comp] = minPt[comp]
            for comp in range(3):
                if maxPt[comp] > total_max[comp]:
                    total_max[comp] = maxPt[comp]

    writepipebuf(struct.pack('fff', total_min[0], total_min[1], total_min[2]))
    writepipebuf(struct.pack('fff', total_max[0], total_max[1], total_max[2]))

def shader_model_update(self, context):
    value = 0.0
    if self.hecl_shader_model == 'PBR':
        value = 1.0
    bloom_value = 0.0
    if self.hecl_mp3_bloom:
        bloom_value = 1.0
    for shad in ('RetroShader', 'RetroDynamicShader', 'RetroDynamicAlphaShader', 'RetroDynamicCharacterShader'):
        if shad in bpy.data.node_groups and 'NewShaderModel' in bpy.data.node_groups[shad].nodes:
            bpy.data.node_groups[shad].nodes['NewShaderModel'].outputs[0].default_value = value
    for shad in ('RetroShaderMP3',):
        if shad in bpy.data.node_groups and 'Mix Shader' in bpy.data.node_groups[shad].nodes:
            bpy.data.node_groups[shad].nodes['Mix Shader'].inputs[0].default_value = bloom_value

# Load scene callback
from bpy.app.handlers import persistent
@persistent
def scene_loaded(dummy):
    # Hide everything from an external library
    if bpy.context.scene.hecl_type != 'FRAME':
        for o in bpy.context.scene.objects:
            if o.library or (o.data and o.data.library):
                o.hide_set(True)

    # Show PATH library objects as wireframes
    if bpy.context.scene.hecl_type == 'PATH':
        if bpy.context.scene.background_set:
            for o in bpy.context.scene.background_set.objects:
                o.display_type = 'WIRE'
        if bpy.context.scene.hecl_path_obj in bpy.context.scene.objects:
            path_obj = bpy.context.scene.objects[bpy.context.scene.hecl_path_obj]
            path_obj.show_wire = True

    # Linked-Child Detection
    for scene in bpy.data.scenes:
        if scene.hecl_type == 'ACTOR':
            actor_data = scene.hecl_sact_data
            for subtype in actor_data.subtypes:
                if subtype.linked_mesh in bpy.data.objects:
                    mesh_obj = bpy.data.objects[subtype.linked_mesh]
                    if subtype.linked_armature in bpy.data.objects:
                        arm_obj = bpy.data.objects[subtype.linked_armature]
                        parent_armature(mesh_obj, arm_obj)
                        for overlay in subtype.overlays:
                            if overlay.linked_mesh in bpy.data.objects:
                                mesh_obj = bpy.data.objects[overlay.linked_mesh]
                                parent_armature(mesh_obj, arm_obj)

    # Show only the active mesh and action
    if sact.SACTSubtype.SACTSubtype_load.poll(bpy.context):
        bpy.ops.scene.sactsubtype_load()
    if sact.SACTAction.SACTAction_load.poll(bpy.context):
        bpy.ops.scene.sactaction_load()

    shader_model_update(bpy.context.scene, bpy.context)

def power_of_distance(context, light, dist):
    color = light.color
    return dist * dist * context.scene.eevee.light_threshold / max(color[0], max(color[1], color[2]))

def power_of_coefficients(context, light):
    epsilon = 1.19e-07
    if light.hecl_falloff_linear < epsilon and light.hecl_falloff_quadratic < epsilon:
        return 0.0
    color = light.color
    intens = max(color[0], max(color[1], color[2]))
    if light.hecl_falloff_quadratic > epsilon:
        if intens <= epsilon:
            return 0.0
        return power_of_distance(context, light, math.sqrt(intens / (0.0588235 * light.hecl_falloff_quadratic)))
    if light.hecl_falloff_linear > epsilon:
        return power_of_distance(context, light, intens / (0.0588235 * light.hecl_falloff_linear))
    return 0.0

def set_light_falloff(self, context):
    self.energy = power_of_coefficients(context, self)

# Registration
def register():
    register_export_type_enum()
    hmdl.register()
    sact.register()
    srea.register()
    frme.register()
    mapa.register()
    mapu.register()
    path.register()
    armature.register()
    bpy.utils.register_class(hecl_scene_panel)
    bpy.utils.register_class(hecl_light_panel)
    bpy.types.Scene.hecl_auto_select = bpy.props.BoolProperty(name='HECL Auto Select', default=True)
    bpy.types.Light.hecl_falloff_constant =  bpy.props.FloatProperty(
        name="HECL Falloff Constant",
        description="Constant falloff coefficient",
        update=set_light_falloff,
        default=1.0,
        min=0.0)
    bpy.types.Light.hecl_falloff_linear =  bpy.props.FloatProperty(
        name="HECL Falloff Linear",
        description="Linear falloff coefficient",
        update=set_light_falloff,
        default=0.0,
        min=0.0)
    bpy.types.Light.hecl_falloff_quadratic =  bpy.props.FloatProperty(
        name="HECL Falloff Quadratic",
        description="Quadratic falloff coefficient",
        update=set_light_falloff,
        default=0.0,
        min=0.0)
    bpy.types.Scene.hecl_shader_model = bpy.props.EnumProperty(name="HECL Shader Model",
                                        description="Which shader model to use for rendering",
                                        items=[
                                            ('ORIGINAL', "Original", "Close approximation of GameCube materials"),
                                            ('PBR', "PBR", "Hybrid PBR materials replacing original reflection")],
                                        update=shader_model_update,
                                        default='ORIGINAL')
    bpy.types.Scene.hecl_mp3_bloom = bpy.props.BoolProperty(name="HECL View MP3 Bloom",
                                                            description="Preview MP3 bloom factors of model",
                                                            update=shader_model_update,
                                                            default=False)
    bpy.app.handlers.load_post.append(scene_loaded)
    Patching.register()

def unregister():
    bpy.app.handlers.load_post.remove(scene_loaded)
    hmdl.unregister()
    sact.unregister()
    srea.unregister()
    path.unregister()
    bpy.utils.unregister_class(hecl_scene_panel)
    bpy.utils.unregister_class(hecl_light_panel)
    Patching.unregister()

if __name__ == "__main__":
    register()
