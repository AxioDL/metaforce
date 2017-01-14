bl_info = {
    "name": "HECL",
    "author": "Jack Andersen <jackoalan@gmail.com>",
    "version": (1, 0),
    "blender": (2, 78),
    "tracker_url": "https://github.com/AxioDL/hecl/issues/new",
    "location": "Properties > Scene > HECL",
    "description": "Enables blender to gather meshes, materials, and textures for hecl",
    "category": "System"}

# Package import
from . import hmdl, sact, srea, swld, mapa, mapu, frme, Nodegrid, Patching
Nodegrid = Nodegrid.Nodegrid
import bpy, os, sys
from bpy.app.handlers import persistent


# Appendable list allowing external addons to register additional resource types
hecl_typeS = [
('NONE', "None", "Active scene not using HECL", None),
('MESH', "Mesh", "Active scene represents an HMDL Mesh", hmdl.draw),
('ACTOR', "Actor", "Active scene represents a HECL Actor", sact.draw),
('AREA', "Area", "Active scene represents a HECL Area", srea.draw),
('WORLD', "World", "Active scene represents a HECL World", swld.draw),
('MAPAREA', "Map Area", "Active scene represents a HECL Map Area", mapa.draw),
('MAPUNIVERSE', "Map Universe", "Active scene represents a HECL Map Universe", mapu.draw),
('FRAME', "Gui Frame", "Active scene represents a HECL Gui Frame", frme.draw)]

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

        for exp_type in hecl_typeS:
            if exp_type[0] == context.scene.hecl_type and callable(exp_type[3]):
                exp_type[3](self.layout, context)
                break


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

# Load scene callback
from bpy.app.handlers import persistent
@persistent
def scene_loaded(dummy):
    # Hide everything from an external library
    for o in bpy.context.scene.objects:
        if o.library:
            o.hide = True

    # Linked-Child Detection
    for scene in bpy.data.scenes:
        if scene.hecl_type == 'ACTOR':
            actor_data = scene.hecl_sact_data
            for subtype in actor_data.subtypes:
                if subtype.linked_mesh in bpy.data.objects:
                    mesh_obj = bpy.data.objects[subtype.linked_mesh]
                    if subtype.linked_armature in bpy.data.objects:
                        arm_obj = bpy.data.objects[subtype.linked_armature]
                        mesh_obj.parent = arm_obj
                        mesh_obj.parent_type = 'ARMATURE'
                        for overlay in subtype.overlays:
                            if overlay.linked_mesh in bpy.data.objects:
                                mesh_obj = bpy.data.objects[overlay.linked_mesh]
                                mesh_obj.parent = arm_obj
                                mesh_obj.parent_type = 'ARMATURE'


    # Show only the active mesh and action
    if sact.SACTSubtype.SACTSubtype_load.poll(bpy.context):
        bpy.ops.scene.sactsubtype_load()
    if sact.SACTAction.SACTAction_load.poll(bpy.context):
        bpy.ops.scene.sactaction_load()


# Registration
def register():
    register_export_type_enum()
    hmdl.register()
    sact.register()
    srea.register()
    frme.register()
    bpy.utils.register_class(hecl_scene_panel)
    bpy.types.Scene.hecl_auto_select = bpy.props.BoolProperty(name='HECL Auto Select', default=True)
    bpy.app.handlers.load_post.append(scene_loaded)
    Patching.register()

def unregister():
    bpy.app.handlers.load_post.remove(scene_loaded)
    hmdl.unregister()
    sact.unregister()
    srea.unregister()
    bpy.utils.unregister_class(hecl_scene_panel)
    Patching.unregister()

if __name__ == "__main__":
    register()
