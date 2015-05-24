import bpy

# Subtype update (if anything important changes)
def active_subtype_update(self, context):
    if context.scene.hecl_type == 'ACTOR' and context.scene.hecl_auto_select:
        if SACTSubtype_load.poll(context):
            bpy.ops.scene.SACTSubtype_load()



# Actor subtype class
class SACTSubtype(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Actor Mesh Name")
    linked_armature = bpy.props.StringProperty(name="Linked Armature Object Source", update=active_subtype_update)
    linked_mesh = bpy.props.StringProperty(name="Linked Mesh Object Source", update=active_subtype_update)


# Panel draw
def draw(layout, context):
    actor_data = context.scene.hecl_sact_data

    row = layout.row(align=True)
    row.alignment = 'LEFT'
    row.prop(actor_data, 'show_subtypes', text="Subtypes", icon='MESH_DATA', emboss=False)
    if actor_data.show_subtypes:

        row = layout.row()
        row.template_list("UI_UL_list", "SCENE_UL_SACTSubtypes",
                          actor_data, 'subtypes', actor_data, 'active_subtype')
        col = row.column(align=True)
        col.operator("scene.sactsubtype_add", icon="ZOOMIN", text="")
        col.operator("scene.sactsubtype_remove", icon="ZOOMOUT", text="")

        if len(actor_data.subtypes) and actor_data.active_subtype >= 0:
            subtype = actor_data.subtypes[actor_data.active_subtype]

            # Load subtype operator
            if not bpy.context.scene.hecl_auto_select:
                layout.operator("scene.sactsubtype_load", icon='FILE_TICK', text="Load Subtype")

            # Name edit field
            layout.prop(subtype, 'name', text="Name")


            # Link external armature search
            layout.prop_search(subtype, 'linked_armature', bpy.data, 'objects', text="Armature")
            linked_armature = None
            if subtype.linked_armature in bpy.data.objects:
                linked_armature = bpy.data.objects[subtype.linked_armature]

            # Validate
            if linked_armature is None:
                layout.label("Source armature not set", icon='ERROR')
            elif linked_armature is not None and linked_armature.type != 'ARMATURE':
                layout.label("Source armature is not an 'ARMATURE'", icon='ERROR')


            # Link external mesh search
            layout.prop_search(subtype, 'linked_mesh', bpy.data, 'objects', text="Mesh")
            linked_mesh = None
            if subtype.linked_mesh in bpy.data.objects:
                linked_mesh = bpy.data.objects[subtype.linked_mesh]

            # Validate
            if linked_mesh is None:
                layout.label("Source mesh not set", icon='ERROR')
            elif linked_mesh.type != 'MESH':
                layout.label("Source mesh not 'MESH'", icon='ERROR')
            elif linked_armature is not None and linked_mesh not in linked_armature.children:
                layout.label(linked_mesh.name+" not a child of "+linked_armature.name, icon='ERROR')
            elif linked_mesh.parent_type != 'ARMATURE':
                layout.label("Source mesh not 'ARMATURE' parent type", icon='ERROR')


# Subtype 'add' operator
class SACTSubtype_add(bpy.types.Operator):
    bl_idname = "scene.sactsubtype_add"
    bl_label = "New HECL Actor Subtype"
    bl_description = "Add New HECL Actor Subtype to active scene"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR')

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        mesh_name = 'ActorMesh'
        if mesh_name in actor_data.subtypes:
            mesh_name = 'ActorMesh.001'
            mesh_idx = 1
            while mesh_name in actor_data.subtypes:
                mesh_idx += 1
                mesh_name = 'ActorMesh.{:0>3}'.format(mesh_idx)
        mesh = actor_data.subtypes.add()
        mesh.name = mesh_name
        actor_data.active_subtype = len(actor_data.subtypes)-1

        return {'FINISHED'}

# Subtype 'remove' operator
class SACTSubtype_remove(bpy.types.Operator):
    bl_idname = "scene.sactsubtype_remove"
    bl_label = "Remove HECL Actor Subtype"
    bl_description = "Remove HECL Actor Subtype from active scene"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR' and
                actor_data.active_subtype >= 0 and
                len(actor_data.subtypes))

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        actor_data.subtypes.remove(actor_data.active_subtype)
        actor_data.active_subtype -= 1
        if actor_data.active_subtype == -1:
            actor_data.active_subtype = 0
        return {'FINISHED'}


# Subtype 'load' operator
class SACTSubtype_load(bpy.types.Operator):
    bl_idname = "scene.sactsubtype_load"
    bl_label = "Load HECL Actor Subtype"
    bl_description = "Loads Subtype for viewing in active scene"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and
                context.scene.hecl_type == 'ACTOR' and
                len(context.scene.hecl_sact_data.subtypes) and
                context.scene.hecl_sact_data.active_subtype >= 0)

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        subtype = actor_data.subtypes[actor_data.active_subtype]

        # Armature
        linked_armature = None
        if subtype.linked_armature in bpy.data.objects:
            linked_armature = bpy.data.objects[subtype.linked_armature]
        else:
            return {'FINISHED'}

        # Hide armature children
        for object in linked_armature.children:
            if object.name in context.scene.objects:
                object.hide = True

        # Hide all meshes
        for mesh_name in actor_data.subtypes:
            if mesh_name.linked_mesh in bpy.data.objects:
                mesh = bpy.data.objects[mesh_name.linked_mesh]
                if mesh.name in context.scene.objects:
                    mesh.hide = True

        # Show only the chosen subtype
        if subtype.linked_mesh in bpy.data.objects:
            mesh_obj = bpy.data.objects[subtype.linked_mesh]
            mesh_obj.hide = False
            if mesh_obj != linked_armature:
                mesh_obj.parent = linked_armature
                mesh_obj.parent_type = 'ARMATURE'

        return {'FINISHED'}


# Registration
def register():
    bpy.utils.register_class(SACTSubtype)
    bpy.utils.register_class(SACTSubtype_add)
    bpy.utils.register_class(SACTSubtype_remove)
    bpy.utils.register_class(SACTSubtype_load)

def unregister():
    bpy.utils.unregister_class(SACTSubtype)
    bpy.utils.unregister_class(SACTSubtype_add)
    bpy.utils.unregister_class(SACTSubtype_remove)
    bpy.utils.unregister_class(SACTSubtype_load)
