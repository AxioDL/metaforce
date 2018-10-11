import bpy

# Subtype update (if anything important changes)
def active_subtype_update(self, context):
    if context.scene.hecl_type == 'ACTOR' and context.scene.hecl_auto_select:
        if SACTSubtype_load.poll(context):
            bpy.ops.scene.sactsubtype_load()


# Actor subtype overlay class
class SACTSubtypeOverlay(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Overlay Name")
    linked_mesh = bpy.props.StringProperty(name="Linked Mesh Object Source", update=active_subtype_update)
    show_overlay = bpy.props.BoolProperty(name="Show Overlay Mesh", update=active_subtype_update)

# Actor attachment class
class SACTAttachment(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Attachment Name")
    linked_armature = bpy.props.StringProperty(name="Linked Armature Object Source", update=active_subtype_update)
    linked_mesh = bpy.props.StringProperty(name="Linked Mesh Object Source", update=active_subtype_update)
    show_attachment = bpy.props.BoolProperty(name="Show Attachment Mesh", update=active_subtype_update)

# Actor subtype class
class SACTSubtype(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Actor Mesh Name")
    linked_armature = bpy.props.StringProperty(name="Linked Armature Object Source", update=active_subtype_update)
    linked_mesh = bpy.props.StringProperty(name="Linked Mesh Object Source", update=active_subtype_update)
    show_mesh = bpy.props.BoolProperty(name="Show Mesh", default=True, update=active_subtype_update)

    overlays =\
    bpy.props.CollectionProperty(type=SACTSubtypeOverlay, name="Subtype Overlay List")
    active_overlay =\
    bpy.props.IntProperty(name="Active Subtype Overlay", default=0, update=active_subtype_update)

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
            layout.prop(subtype, 'show_mesh', text="Show Mesh")

            # Mesh overlays
            layout.label("Overlay Meshes:")
            row = layout.row()
            row.template_list("UI_UL_list", "SCENE_UL_SACTSubtypeOverlays",
                              subtype, 'overlays', subtype, 'active_overlay')
            col = row.column(align=True)
            col.operator("scene.sactsubtypeoverlay_add", icon="ZOOMIN", text="")
            col.operator("scene.sactsubtypeoverlay_remove", icon="ZOOMOUT", text="")

            overlay_mesh = None
            if len(subtype.overlays) and subtype.active_overlay >= 0:
                overlay = subtype.overlays[subtype.active_overlay]
                layout.prop(overlay, 'name', text="Name")
                layout.prop_search(overlay, 'linked_mesh', bpy.data, 'objects', text="Mesh")
                if overlay.linked_mesh in bpy.data.objects:
                    overlay_mesh = bpy.data.objects[overlay.linked_mesh]
                layout.prop(overlay, 'show_overlay', text="Show Overlay")

            # Mesh attachments
            layout.label("Attachment Meshes:")
            row = layout.row()
            row.template_list("UI_UL_list", "SCENE_UL_SACTAttachments",
                              actor_data, 'attachments', actor_data, 'active_attachment')
            col = row.column(align=True)
            col.operator("scene.sactattachment_add", icon="ZOOMIN", text="")
            col.operator("scene.sactattachment_remove", icon="ZOOMOUT", text="")

            attachment_armature = linked_armature
            attachment_mesh = None
            if len(actor_data.attachments) and actor_data.active_attachment >= 0:
                attachment = actor_data.attachments[actor_data.active_attachment]
                layout.prop(attachment, 'name', text="Name")
                layout.prop_search(attachment, 'linked_armature', bpy.data, 'objects', text="Armature")
                if attachment.linked_armature in bpy.data.objects:
                    attachment_armature = bpy.data.objects[attachment.linked_armature]
                layout.prop_search(attachment, 'linked_mesh', bpy.data, 'objects', text="Mesh")
                if attachment.linked_mesh in bpy.data.objects:
                    attachment_mesh = bpy.data.objects[attachment.linked_mesh]
                layout.prop(attachment, 'show_attachment', text="Show Attachment")

            # Validate
            if linked_mesh is None:
                layout.label("Source mesh not set", icon='ERROR')
            elif linked_mesh.type != 'MESH':
                layout.label("Source mesh not 'MESH'", icon='ERROR')
            elif linked_armature is not None and linked_mesh not in linked_armature.children:
                layout.label(linked_mesh.name+" not a child of "+linked_armature.name, icon='ERROR')
            elif linked_mesh.parent_type != 'ARMATURE':
                layout.label("Source mesh not 'ARMATURE' parent type", icon='ERROR')

            if overlay_mesh:
                if overlay_mesh.type != 'MESH':
                    layout.label("Overlay mesh not 'MESH'", icon='ERROR')
                elif overlay_mesh.parent_type != 'ARMATURE':
                    layout.label("Overlay mesh not 'ARMATURE' parent type", icon='ERROR')

            if attachment_mesh:
                if attachment_mesh.type != 'MESH':
                    layout.label("Attachment mesh not 'MESH'", icon='ERROR')
                elif attachment_armature is not None and attachment_mesh not in attachment_armature.children:
                    layout.label(attachment_mesh.name+" not a child of "+attachment_armature.name, icon='ERROR')
                elif attachment_mesh.parent_type != 'ARMATURE':
                    layout.label("Attachment mesh not 'ARMATURE' parent type", icon='ERROR')


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

        # Hide all meshes (incl overlays)
        for subtype_data in actor_data.subtypes:
            if subtype_data.linked_mesh in bpy.data.objects:
                mesh = bpy.data.objects[subtype_data.linked_mesh]
                if mesh.name in context.scene.objects:
                    mesh.hide = True
            for overlay in subtype_data.overlays:
                if overlay.linked_mesh in bpy.data.objects:
                    mesh = bpy.data.objects[overlay.linked_mesh]
                    if mesh.name in context.scene.objects:
                        mesh.hide = True

        # Hide/Show selected attachment meshes
        for attachment in actor_data.attachments:
            if attachment.linked_mesh in bpy.data.objects:
                mesh_obj = bpy.data.objects[attachment.linked_mesh]
                if mesh_obj.name in context.scene.objects:
                    mesh_obj.hide = not attachment.show_attachment
                attachment_armature = linked_armature
                if attachment.linked_armature in bpy.data.objects:
                    attachment_armature = bpy.data.objects[attachment.linked_armature]
                if mesh_obj != attachment_armature:
                    mesh_obj.parent = attachment_armature
                    mesh_obj.parent_type = 'ARMATURE'

        # Show only the chosen subtype (and selected overlays)
        if subtype.linked_mesh in bpy.data.objects:
            mesh_obj = bpy.data.objects[subtype.linked_mesh]
            if subtype.show_mesh:
                mesh_obj.hide = False
            if mesh_obj != linked_armature:
                mesh_obj.parent = linked_armature
                mesh_obj.parent_type = 'ARMATURE'
            for overlay in subtype.overlays:
                if overlay.linked_mesh in bpy.data.objects:
                    mesh_obj = bpy.data.objects[overlay.linked_mesh]
                    if overlay.show_overlay:
                        mesh_obj.hide = False
                    if mesh_obj != linked_armature:
                        mesh_obj.parent = linked_armature
                        mesh_obj.parent_type = 'ARMATURE'

        return {'FINISHED'}


# Subtype overlay 'add' operator
class SACTSubtypeOverlay_add(bpy.types.Operator):
    bl_idname = "scene.sactsubtypeoverlay_add"
    bl_label = "New HECL Actor Subtype Overlay"
    bl_description = "Add New HECL Actor Subtype Overlay"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR' and
                len(actor_data.subtypes) and actor_data.active_subtype >= 0)

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        subtype = actor_data.subtypes[actor_data.active_subtype]
        overlay_name = 'ActorOverlay'
        if overlay_name in subtype.overlays:
            overlay_name = 'ActorOverlay.001'
            overlay_idx = 1
            while overlay_name in subtype.overlays:
                overlay_idx += 1
                overlay_name = 'ActorOverlay.{:0>3}'.format(overlay_idx)
        overlay = subtype.overlays.add()
        overlay.name = overlay_name
        subtype.active_overlay = len(subtype.overlays)-1

        return {'FINISHED'}

# Subtype overlay 'remove' operator
class SACTSubtypeOverlay_remove(bpy.types.Operator):
    bl_idname = "scene.sactsubtypeoverlay_remove"
    bl_label = "Remove HECL Actor Subtype Overlay"
    bl_description = "Remove HECL Actor Subtype Overlay"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR' and
                actor_data.active_subtype >= 0 and
                len(actor_data.subtypes) and
                actor_data.subtypes[actor_data.active_subtype].active_overlay >= 0 and
                len(actor_data.subtypes[actor_data.active_subtype].overlays))

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        subtype = actor_data.subtypes[actor_data.active_subtype]
        subtype.overlays.remove(subtype.active_overlay)
        subtype.active_overlay -= 1
        if subtype.active_overlay == -1:
            subtype.active_overlay = 0
        return {'FINISHED'}

# Subtype overlay 'add' operator
class SACTAttachment_add(bpy.types.Operator):
    bl_idname = "scene.sactattachment_add"
    bl_label = "New HECL Actor Attachment"
    bl_description = "Add New HECL Actor Attachment"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR')

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        attachment_name = 'ActorAttachment'
        if attachment_name in actor_data.attachments:
            attachment_name = 'ActorAttachment.001'
            attachment_idx = 1
            while attachment_name in actor_data.attachments:
                attachment_idx += 1
                attachment_name = 'ActorAttachment.{:0>3}'.format(attachment_idx)
        attachment = actor_data.attachments.add()
        attachment.name = attachment_name
        actor_data.active_attachment = len(actor_data.attachments)-1

        return {'FINISHED'}

# Subtype overlay 'remove' operator
class SACTAttachment_remove(bpy.types.Operator):
    bl_idname = "scene.sactattachment_remove"
    bl_label = "Remove HECL Actor Attachment"
    bl_description = "Remove HECL Actor Attachment"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR' and
                actor_data.active_attachment >= 0 and
                len(actor_data.attachments))

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        actor_data.attachments.remove(actor_data.active_attachment)
        actor_data.active_attachment -= 1
        if actor_data.active_attachment == -1:
            actor_data.active_attachment = 0
        return {'FINISHED'}

# Registration
def register():
    bpy.utils.register_class(SACTSubtypeOverlay)
    bpy.utils.register_class(SACTSubtypeOverlay_add)
    bpy.utils.register_class(SACTSubtypeOverlay_remove)
    bpy.utils.register_class(SACTAttachment)
    bpy.utils.register_class(SACTAttachment_add)
    bpy.utils.register_class(SACTAttachment_remove)
    bpy.utils.register_class(SACTSubtype)
    bpy.utils.register_class(SACTSubtype_add)
    bpy.utils.register_class(SACTSubtype_remove)
    bpy.utils.register_class(SACTSubtype_load)

def unregister():
    bpy.utils.unregister_class(SACTSubtype)
    bpy.utils.unregister_class(SACTSubtype_add)
    bpy.utils.unregister_class(SACTSubtype_remove)
    bpy.utils.unregister_class(SACTSubtype_load)
    bpy.utils.unregister_class(SACTAttachment)
    bpy.utils.unregister_class(SACTAttachment_add)
    bpy.utils.unregister_class(SACTAttachment_remove)
    bpy.utils.unregister_class(SACTSubtypeOverlay)
    bpy.utils.unregister_class(SACTSubtypeOverlay_add)
    bpy.utils.unregister_class(SACTSubtypeOverlay_remove)
