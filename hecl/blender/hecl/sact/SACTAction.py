import bpy

# Action update (if anything important changes)
def active_action_update(self, context):
    if not bpy.app.background:
        if context.scene.hecl_type == 'ACTOR' and context.scene.hecl_auto_select:
            if SACTAction_load.poll(context):
                bpy.ops.scene.sactaction_load()

# Action type update
def action_type_update(self, context):
    if not bpy.app.background:
        actor_data = context.scene.hecl_sact_data
        active_action_update(self, context)

# Actor action class
class SACTAction(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Action Name")

# Panel draw
def draw(layout, context):
    actor_data = context.scene.hecl_sact_data

    row = layout.row(align=True)
    row.alignment = 'LEFT'
    row.prop(actor_data, 'show_actions', text="Actions", icon='ACTION', emboss=False)
    if actor_data.show_actions:

        row = layout.row()
        row.template_list("UI_UL_list", "SCENE_UL_SACTActions",
                          actor_data, 'actions', actor_data, 'active_action')
        col = row.column(align=True)
        col.operator("scene.sactaction_add", icon="ZOOMIN", text="")
        col.operator("scene.sactaction_remove", icon="ZOOMOUT", text="")

        if len(actor_data.actions) and actor_data.active_action >= 0:
            action = actor_data.actions[actor_data.active_action]

            # Load action operator
            if not bpy.context.scene.hecl_auto_select:
                layout.operator("scene.sactaction_load", icon='FILE_TICK', text="Load Action")

            # Name edit field
            layout.prop_search(action, 'name', bpy.data, 'actions', text="Name")
            linked_action = None
            if bpy.data.actions.find(action.name) != -1:
                linked_action = bpy.data.actions[action.name]

            # Validate
            if linked_action is None:
                layout.label("Source action not set", icon='ERROR')
            else:
                #layout.prop(linked_action, 'hecl_index', text="Index")
                #layout.prop(linked_action, 'hecl_anim_props', text="Props")
                layout.prop(linked_action, 'hecl_fps', text="Frame Rate")
                row = layout.row()
                row.prop(context.scene, 'hecl_auto_remap', text="60-fps Remap")
                row.prop(linked_action, 'hecl_additive', text="Additive")
                #row.prop(linked_action, 'hecl_looping', text="Looping")



# Action 'add' operator
class SACTAction_add(bpy.types.Operator):
    bl_idname = "scene.sactaction_add"
    bl_label = "New HECL Actor Action"
    bl_description = "Add New HECL Actor Action to active scene"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR')

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        action_name = 'ActorAction'
        if action_name in actor_data.actions:
            action_name = 'ActorAction.001'
            action_idx = 1
            while action_name in actor_data.actions:
                action_idx += 1
                action_name = 'ActorAction.{:0>3}'.format(action_idx)
        action = actor_data.actions.add()
        action.name = action_name
        actor_data.active_action = len(actor_data.actions)-1

        return {'FINISHED'}

# Action 'remove' operator
class SACTAction_remove(bpy.types.Operator):
    bl_idname = "scene.sactaction_remove"
    bl_label = "Remove HECL Actor Action"
    bl_description = "Remove HECL Actor Action from active scene"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        return (context.scene is not None and
                not context.scene.library and
                context.scene.hecl_type == 'ACTOR' and
                actor_data.active_action >= 0 and
                len(actor_data.actions))

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        actor_data.actions.remove(actor_data.active_action)
        actor_data.active_action -= 1
        if actor_data.active_action == -1:
            actor_data.active_action = 0
        return {'FINISHED'}


# Action 'load' operator
class SACTAction_load(bpy.types.Operator):
    bl_idname = "scene.sactaction_load"
    bl_label = "Load HECL Actor Action"
    bl_description = "Loads Action for playback in active scene"

    @classmethod
    def poll(cls, context):
        return (context.scene is not None and
                context.scene.hecl_type == 'ACTOR' and
                len(context.scene.hecl_sact_data.actions) and
                context.scene.hecl_sact_data.active_action >= 0)

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data

        if actor_data.active_action not in range(len(actor_data.actions)):
            return {'CANCELLED'}
        if actor_data.active_subtype not in range(len(actor_data.subtypes)):
            return {'CANCELLED'}

        action_data = actor_data.actions[actor_data.active_action]
        subtype = actor_data.subtypes[actor_data.active_subtype]

        # Refresh event markers
        #SACTEvent.clear_event_markers(actor_data, context)
        #SACTEvent.update_action_events(None)
        #SACTEvent.active_event_update(None, context)

        # Clear animation data for all subtypes
        for s in range(len(actor_data.subtypes)):
            st = actor_data.subtypes[s]
            if st.linked_armature in bpy.data.objects:
                am = bpy.data.objects[st.linked_armature]
                am.animation_data_clear()

        # Set single action into armature
        if subtype.linked_armature in bpy.data.objects:
            armature_obj = bpy.data.objects[subtype.linked_armature]

            for bone in armature_obj.pose.bones:
                bone.location = (0,0,0)
                bone.rotation_quaternion = (1,0,0,0)
                bone.scale = (1,1,1)

            if action_data.name in bpy.data.actions:
                action_obj =\
                bpy.data.actions[action_data.name]
                armature_obj.animation_data_clear()
                armature_obj.animation_data_create()
                armature_obj.animation_data.action = action_obj

                # Time remapping
                if context.scene.hecl_auto_remap:
                    bpy.context.scene.render.fps = 60
                    bpy.context.scene.render.frame_map_old = action_obj.hecl_fps
                    bpy.context.scene.render.frame_map_new = 60
                    bpy.context.scene.frame_start = 0
                    bpy.context.scene.frame_end = action_obj.frame_range[1] * (60 / action_obj.hecl_fps)
                else:
                    bpy.context.scene.render.fps = action_obj.hecl_fps
                    bpy.context.scene.render.frame_map_old = action_obj.hecl_fps
                    bpy.context.scene.render.frame_map_new = action_obj.hecl_fps
                    bpy.context.scene.frame_start = 0
                    bpy.context.scene.frame_end = action_obj.frame_range[1]

                # Events
                #SACTEvent.clear_action_events(self, context, actor_data)
                #SACTEvent.load_action_events(self, context, action_obj, 0)

                return {'FINISHED'}

            else:
                armature_obj.animation_data_clear()
                self.report({'WARNING'}, "Unable to load action; check HECL panel")
                return {'FINISHED'}

        else:
            self.report({'WARNING'}, "Unable to load armature; check HECL panel")
            return {'FINISHED'}




# Registration
def register():
    bpy.types.Action.hecl_fps = bpy.props.IntProperty(name="HECL Actor Sub-action Frame-rate",
                                                      description="Frame-rate at which action is authored; to be interpolated at 60-fps by runtime",
                                                      min=1, max=60, default=30,
                                                      update=active_action_update)
    bpy.utils.register_class(SACTAction)
    bpy.utils.register_class(SACTAction_add)
    bpy.utils.register_class(SACTAction_load)
    bpy.utils.register_class(SACTAction_remove)


def unregister():
    bpy.utils.unregister_class(SACTAction)
    bpy.utils.unregister_class(SACTAction_add)
    bpy.utils.unregister_class(SACTAction_load)
    bpy.utils.unregister_class(SACTAction_remove)
