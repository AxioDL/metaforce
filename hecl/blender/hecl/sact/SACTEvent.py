import bpy

# Loop event class
class hecl_actor_event_loop(bpy.types.PropertyGroup):
    bool = bpy.props.BoolProperty(name="Loop Bool")

# UEVT event class
class hecl_actor_event_uevt(bpy.types.PropertyGroup):
    type = bpy.props.IntProperty(name="UEVT Type")
    bone_name = bpy.props.StringProperty(name="Bone Name")

# Effect event class
class hecl_actor_event_effect(bpy.types.PropertyGroup):
    frame_count = bpy.props.IntProperty(name="Frame Count", min=0)
    uid = bpy.props.StringProperty(name="Effect UID")
    bone_name = bpy.props.StringProperty(name="Bone Name")
    scale = bpy.props.FloatProperty(name="Scale", description="Proportional spacial scale")
    transform_mode = bpy.props.EnumProperty(name="Transform Mode", description="How the bone will transform the effect",
                                            items=[('STATIONARY', "Stationary", "Effect emitter will be transformed in bone-space, then retained"),
                                                   ('WORLD', "World", "Effect emitter will be transformed in bone-space"),
                                                   ('LOCAL', "Local", "Entire effect will be transformed in bone-space")])

# Sound event class
class hecl_actor_event_sound(bpy.types.PropertyGroup):
    sound_id = bpy.props.StringProperty(name="Sound ID")
    ref_amp = bpy.props.FloatProperty(name="Reference Amplitude")
    ref_dist = bpy.props.FloatProperty(name="Reference Distance")

# Name update
def update_name(self, context):
    if bpy.context.scene.hecl_type == 'ACTOR':
        clear_event_markers(bpy.context.scene.hecl_sact_data, context)
        update_action_events(None)
        active_event_update(self, context)


# Actor event class
class hecl_actor_event(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Event Name",
                                    update=update_name)
    type = bpy.props.EnumProperty(name="Event Type",
                                  items=[('LOOP', "Loop", "Loop Event"),
                                         ('UEVT', "UEVT", "UEVT Event"),
                                         ('EFFECT', "Effect", "Effect Event"),
                                         ('SOUND', "Sound", "Sound Event")],
                                  default='LOOP')

    index = bpy.props.IntProperty(name="Event Index")
    time = bpy.props.FloatProperty(name="Event Time")
    props = bpy.props.StringProperty(name="Event props")

    loop_data = bpy.props.PointerProperty(name="Loop event data",
                                          type=hecl_actor_event_loop)
    uevt_data = bpy.props.PointerProperty(name="UEVT event data",
                                          type=hecl_actor_event_uevt)
    effect_data = bpy.props.PointerProperty(name="Effect event data",
                                            type=hecl_actor_event_effect)
    sound_data = bpy.props.PointerProperty(name="Sound event data",
                                           type=hecl_actor_event_sound)


# Panel draw
def draw(layout, context):
    actor_data = context.scene.hecl_sact_data

    armature = None
    if actor_data.active_subtype >= 0:
        if actor_data.active_subtype in range(len(actor_data.subtypes)):
            subtype = actor_data.subtypes[actor_data.active_subtype]
            if subtype and subtype.linked_armature in bpy.data.objects:
                armature = bpy.data.objects[subtype.linked_armature]

    row = layout.row(align=True)
    row.alignment = 'LEFT'
    row.prop(actor_data, 'show_events', text="Events", icon='PREVIEW_RANGE', emboss=False)
    if actor_data.show_events:

        # Get action
        action_data = None
        subaction_data = None
        if actor_data.active_action in range(len(actor_data.actions)):
            action_data = actor_data.actions[actor_data.active_action]
            if action_data.type == 'SINGLE':
                subaction_data = action_data.subactions[0]
            elif action_data.type == 'SEQUENCE' or action_data.type == 'RANDOM':
                if action_data.active_subaction in range(len(action_data.subactions)):
                    subaction_data = action_data.subactions[action_data.active_subaction]

        # Validate
        if subaction_data is None:
            layout.label("(Sub)action not selected in 'Actions'", icon='ERROR')
        else:

            if subaction_data.name == '':
                layout.label("Action not set", icon='ERROR')
            elif subaction_data.name not in bpy.data.actions:
                layout.label("Action '"+subaction_data.name+"' not found", icon='ERROR')
            else:
                action = bpy.data.actions[subaction_data.name]

                # Event list
                row = layout.row()
                row.template_list("UI_UL_list", "SCENE_UL_hecl_actor_subaction_events",
                                  action, 'hecl_events', action, 'hecl_active_event')
                col = row.column(align=True)
                col.operator("scene.hecl_actor_subaction_event_add", icon="ZOOMIN", text="")
                col.operator("scene.hecl_actor_subaction_event_remove", icon="ZOOMOUT", text="")
                col.separator()
                col.operator("scene.hecl_actor_subaction_event_move_up", icon="TRIA_UP", text="")
                col.operator("scene.hecl_actor_subaction_event_move_down", icon="TRIA_DOWN", text="")


                if len(action.hecl_events) and action.hecl_active_event >= 0:
                    event = action.hecl_events[action.hecl_active_event]

                    layout.prop(event, 'name', text="Name")
                    layout.prop(event, 'index', text="Index")
                    layout.prop(event, 'props', text="Props")
                    layout.label('Marker Time: ' + '{:g}'.format(event.time), icon='MARKER_HLT')

                    layout.label("Event Type:")
                    row = layout.row(align=True)
                    row.prop_enum(event, 'type', 'LOOP')
                    row.prop_enum(event, 'type', 'UEVT')
                    row.prop_enum(event, 'type', 'EFFECT')
                    row.prop_enum(event, 'type', 'SOUND')

                    if event.type == 'LOOP':
                        loop_data = event.loop_data
                        layout.prop(loop_data, 'bool')

                    elif event.type == 'UEVT':
                        uevt_data = event.uevt_data
                        layout.prop(uevt_data, 'type')
                        layout.prop(uevt_data, 'bone_name')

                    elif event.type == 'EFFECT':
                        effect_data = event.effect_data
                        layout.prop(effect_data, 'frame_count')
                        layout.prop(effect_data, 'uid')
                        if armature:
                            layout.prop_search(effect_data, 'bone_name', armature.data, 'bones')
                        else:
                            layout.prop(effect_data, 'bone_name')
                        layout.prop(effect_data, 'scale')
                        row = layout.row(align=True)
                        row.prop_enum(effect_data, 'transform_mode', 'STATIONARY')
                        row.prop_enum(effect_data, 'transform_mode', 'WORLD')
                        row.prop_enum(effect_data, 'transform_mode', 'LOCAL')

                    elif event.type == 'SOUND':
                        sound_data = event.sound_data
                        layout.prop(sound_data, 'sound_id')
                        layout.prop(sound_data, 'ref_amp')
                        layout.prop(sound_data, 'ref_dist')

# Clear event markers not in active event
def clear_event_markers(actor_data, context):
    for marker in context.scene.timeline_markers:
        if marker.name.startswith('hecl_'):
            context.scene.timeline_markers.remove(marker)

# Event marker update
@bpy.app.handlers.persistent
def update_action_events(dummy):
    context = bpy.context
    if context.scene.hecl_type == 'ACTOR':
        actor_data = context.scene.hecl_sact_data

        if actor_data.active_action in range(len(actor_data.actions)):
            action_data = actor_data.actions[actor_data.active_action]
            if action_data.name in bpy.data.actions:
                action_obj =\
                bpy.data.actions[action_data.name]
                for i in range(len(action_obj.hecl_events)):
                    event = action_obj.hecl_events[i]
                    marker_name = 'hecl_' + str(action_obj.hecl_index) + '_' + str(i) + '_' + event.name
                    if marker_name in context.scene.timeline_markers:
                        marker = context.scene.timeline_markers[marker_name]
                        event_time = marker.frame / action_obj.hecl_fps
                        if event_time != event.time:
                            event.time = event_time
                    else:
                        marker = context.scene.timeline_markers.new(marker_name)
                        marker.frame = event.time * action_obj.hecl_fps
                        marker.select = False

                    if i != action_obj.hecl_active_event and marker.select:
                        action_obj.hecl_active_event = i



# Event 'add' operator
class hecl_actor_subaction_event_add(bpy.types.Operator):
    bl_idname = "scene.hecl_actor_subaction_event_add"
    bl_label = "New HECL Actor Event"
    bl_description = "Add New HECL Actor Event to active Sub-action"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        check = (context.scene is not None and
                 not context.scene.library and
                 context.scene.hecl_type == 'ACTOR' and
                 len(actor_data.actions) and
                 actor_data.active_action >= 0 and
                 len(actor_data.actions[actor_data.active_action].subactions) and
                 actor_data.actions[actor_data.active_action].active_subaction >= 0)
        if not check:
            return False
        actor_data = context.scene.hecl_sact_data
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        return subaction_data.name in bpy.data.actions

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        blend_action = bpy.data.actions[subaction_data.name]
        event_name = 'SubactionEvent'
        if event_name in blend_action.hecl_events:
            event_name = 'SubactionEvent.001'
            event_idx = 1
            while event_name in blend_action.hecl_events:
                event_idx += 1
                event_name = 'SubactionEvent.{:0>3}'.format(event_idx)
        event = blend_action.hecl_events.add()
        event.name = event_name
        action_obj =\
        bpy.data.actions[subaction_data.name]
        event.time = (context.scene.frame_current / (context.scene.render.frame_map_new / context.scene.render.frame_map_old)) / action_obj.hecl_fps
        blend_action.hecl_active_event = len(blend_action.hecl_events)-1

        if not bpy.app.background:
            update_action_events(None)
        active_event_update(self, context)

        return {'FINISHED'}

# Event 'remove' operator
class hecl_actor_subaction_event_remove(bpy.types.Operator):
    bl_idname = "scene.hecl_actor_subaction_event_remove"
    bl_label = "Remove HECL Actor Event"
    bl_description = "Remove HECL Actor Event from active Sub-action"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        check = (context.scene is not None and
                 not context.scene.library and
                 context.scene.hecl_type == 'ACTOR' and
                 len(actor_data.actions) and
                 actor_data.active_action >= 0 and
                 len(actor_data.actions[actor_data.active_action].subactions) and
                 actor_data.actions[actor_data.active_action].active_subaction >= 0)
        if not check:
            return False
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        if subaction_data.name not in bpy.data.actions:
            return False
        blend_action = bpy.data.actions[subaction_data.name]
        return blend_action.hecl_active_event in range(len(blend_action.hecl_events))

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        blend_action = bpy.data.actions[subaction_data.name]
        event_name = blend_action.hecl_events[blend_action.hecl_active_event].name
        blend_action.hecl_events.remove(blend_action.hecl_active_event)

        marker_name = 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name
        if marker_name in context.scene.timeline_markers:
            context.scene.timeline_markers.remove(context.scene.timeline_markers[marker_name])

        blend_action.hecl_active_event -= 1
        if blend_action.hecl_active_event == -1:
            blend_action.hecl_active_event = 0

        clear_event_markers(actor_data, context)

        return {'FINISHED'}


# Event 'move down' operator
class hecl_actor_subaction_event_move_down(bpy.types.Operator):
    bl_idname = "scene.hecl_actor_subaction_event_move_down"
    bl_label = "Move HECL Actor Event Down in Stack"
    bl_description = "Move HECL Actor Event down in stack from active Sub-action"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        check = (context.scene is not None and
                 not context.scene.library and
                 context.scene.hecl_type == 'ACTOR' and
                 len(actor_data.actions) and
                 actor_data.active_action >= 0 and
                 len(actor_data.actions[actor_data.active_action].subactions) and
                 actor_data.actions[actor_data.active_action].active_subaction >= 0)
        if not check:
            return False
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        if subaction_data.name not in bpy.data.actions:
            return False
        blend_action = bpy.data.actions[subaction_data.name]
        return (blend_action.hecl_active_event in range(len(blend_action.hecl_events)) and
                blend_action.hecl_active_event < len(blend_action.hecl_events) - 1)

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        blend_action = bpy.data.actions[subaction_data.name]
        event_name_a = blend_action.hecl_events[blend_action.hecl_active_event].name
        event_name_b = blend_action.hecl_events[blend_action.hecl_active_event + 1].name
        blend_action.hecl_events.move(blend_action.hecl_active_event, blend_action.hecl_active_event + 1)

        marker_name_a = 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name_a
        marker_a = None
        if marker_name_a in context.scene.timeline_markers:
            marker_a = context.scene.timeline_markers[marker_name_a]

        marker_name_b = 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event + 1) + '_' + event_name_b
        marker_b = None
        if marker_name_b in context.scene.timeline_markers:
            marker_b = context.scene.timeline_markers[marker_name_b]

        if marker_a and marker_b:
            marker_a.name =\
            'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event + 1) + '_' + event_name_a
            marker_b.name =\
            'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name_b

        blend_action.hecl_active_event += 1

        return {'FINISHED'}


# Event 'move up' operator
class hecl_actor_subaction_event_move_up(bpy.types.Operator):
    bl_idname = "scene.hecl_actor_subaction_event_move_up"
    bl_label = "Move HECL Actor Event Up in Stack"
    bl_description = "Move HECL Actor Event up in stack from active Sub-action"

    @classmethod
    def poll(cls, context):
        actor_data = context.scene.hecl_sact_data
        check = (context.scene is not None and
                 not context.scene.library and
                 context.scene.hecl_type == 'ACTOR' and
                 len(actor_data.actions) and
                 actor_data.active_action >= 0 and
                 len(actor_data.actions[actor_data.active_action].subactions) and
                 actor_data.actions[actor_data.active_action].active_subaction >= 0)
        if not check:
            return False
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        if subaction_data.name not in bpy.data.actions:
            return False
        blend_action = bpy.data.actions[subaction_data.name]
        return (blend_action.hecl_active_event in range(len(blend_action.hecl_events)) and
            blend_action.hecl_active_event > 0)

    def execute(self, context):
        actor_data = context.scene.hecl_sact_data
        action_data = actor_data.actions[actor_data.active_action]
        subaction_data = action_data.subactions[action_data.active_subaction]
        blend_action = bpy.data.actions[subaction_data.name]
        event_name_a = blend_action.hecl_events[blend_action.hecl_active_event].name
        event_name_b = blend_action.hecl_events[blend_action.hecl_active_event - 1].name
        blend_action.hecl_events.move(blend_action.hecl_active_event, blend_action.hecl_active_event - 1)

        marker_name_a = 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name_a
        marker_a = None
        if marker_name_a in context.scene.timeline_markers:
            marker_a = context.scene.timeline_markers[marker_name_a]

        marker_name_b = 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event - 1) + '_' + event_name_b
        marker_b = None
        if marker_name_b in context.scene.timeline_markers:
            marker_b = context.scene.timeline_markers[marker_name_b]

        if marker_a and marker_b:
            marker_a.name =\
            'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event - 1) + '_' + event_name_a
            marker_b.name =\
            'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name_b

        blend_action.hecl_active_event -= 1

        return {'FINISHED'}


def active_event_update(self, context):
    actor_data = context.scene.hecl_sact_data
    if actor_data.active_action in range(len(actor_data.actions)):
        action_data = actor_data.actions[actor_data.active_action]
        for marker in context.scene.timeline_markers:
            if marker.name.startswith('hecl_'):
                blend_action = bpy.data.actions[action_data.name]
                event_name = blend_action.hecl_events[blend_action.hecl_active_event].name
                if marker.name == 'hecl_' + str(blend_action.hecl_index) + '_' + str(blend_action.hecl_active_event) + '_' + event_name:
                    marker.select = True
                else:
                    marker.select = False

# Registration
def register():
    bpy.utils.register_class(hecl_actor_event_loop)
    bpy.utils.register_class(hecl_actor_event_uevt)
    bpy.utils.register_class(hecl_actor_event_effect)
    bpy.utils.register_class(hecl_actor_event_sound)
    bpy.utils.register_class(hecl_actor_event)
    bpy.utils.register_class(hecl_actor_subaction_event_add)
    bpy.utils.register_class(hecl_actor_subaction_event_remove)
    bpy.utils.register_class(hecl_actor_subaction_event_move_down)
    bpy.utils.register_class(hecl_actor_subaction_event_move_up)
    bpy.types.Action.hecl_events = bpy.props.CollectionProperty(name="HECL action event",
                                                                type=hecl_actor_event)
    bpy.types.Action.hecl_active_event = bpy.props.IntProperty(name="HECL active action event",
                                                              default=0,
                                                              update=active_event_update)
    if not bpy.app.background and update_action_events not in bpy.app.handlers.scene_update_post:
        bpy.app.handlers.scene_update_post.append(update_action_events)

def unregister():
    if update_action_events in bpy.app.handlers.scene_update_post:
        bpy.app.handlers.scene_update_post.remove(update_action_events)
    bpy.utils.unregister_class(hecl_actor_event)
    bpy.utils.unregister_class(hecl_actor_event_loop)
    bpy.utils.unregister_class(hecl_actor_event_uevt)
    bpy.utils.unregister_class(hecl_actor_event_effect)
    bpy.utils.unregister_class(hecl_actor_event_sound)
    bpy.utils.unregister_class(hecl_actor_subaction_event_add)
    bpy.utils.unregister_class(hecl_actor_subaction_event_remove)
    bpy.utils.unregister_class(hecl_actor_subaction_event_move_down)
    bpy.utils.unregister_class(hecl_actor_subaction_event_move_up)
