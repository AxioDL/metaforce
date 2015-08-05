'''
Root HECL addon package for Blender
'''

bl_info = {
    "name": "HECL",
    "author": "Jack Andersen <jackoalan@gmail.com>",
    "version": (1, 0),
    "blender": (2, 74),
    "tracker_url": "https://github.com/RetroView/hecl/issues/new",
    "location": "Properties > Scene > HECL",
    "description": "Enables blender to gather meshes, materials, and textures for hecl",
    "category": "System"}

# Package import
from . import hmdl, sact, Nodegrid
Nodegrid = Nodegrid.Nodegrid
import bpy, os, sys
from bpy.app.handlers import persistent

# Appendable list allowing external addons to register additional resource types
hecl_typeS = [
('NONE', "None", "Active scene not using HECL", None, None),
('MESH', "Mesh", "Active scene represents an HMDL Mesh", hmdl.draw, hmdl.cook),
('ACTOR', "Actor", "Active scene represents a HECL Actor", sact.draw, sact.cook)]

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


# Blender-selected polymorphism cook
def do_cook(writebuf, platform_type, endian_char):
    for tp in hecl_typeS:
        if tp[0] == bpy.context.scene.hecl_type:
            if callable(tp[4]):
                return tp[4](writefd, platform_type, endian_char)
    return False


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
    if cmdline[0] == b'COOK':
        resource_type = bpy.context.scene.hecl_type.encode()
        writepipeline(resource_type)
        ackbytes = readpipeline()
        if ackbytes != b'ACK':
            return
        try:
            result = do_cook(writepipebuf, cmdline[1].decode(), cmdline[2].decode())
            if result == None or result == True:
                writepipeline(b'SUCCESS')
            else:
                writepipeline(b'FAILURE')
        except:
            writepipeline(b'EXCEPTION')


# Registration
def register():
    register_export_type_enum()
    hmdl.register()
    sact.register()
    bpy.utils.register_class(hecl_scene_panel)

def unregister():
    hmdl.unregister()
    sact.unregister()
    bpy.utils.unregister_class(hecl_scene_panel)

if __name__ == "__main__":
    register()
