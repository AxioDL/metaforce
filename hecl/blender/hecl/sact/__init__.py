from . import SACTSubtype, SACTAction, ANIM
from .. import armature

import bpy
import bpy.path
import re
import struct
from mathutils import Vector, Quaternion, Euler

# Actor data class
class SACTData(bpy.types.PropertyGroup):

    subtypes: bpy.props.CollectionProperty(type=SACTSubtype.SACTSubtype, name="Actor Subtype List")
    active_subtype: bpy.props.IntProperty(name="Active Actor Subtype", default=0, update=SACTSubtype.active_subtype_update)
    show_subtypes: bpy.props.BoolProperty()

    attachments: bpy.props.CollectionProperty(type=SACTSubtype.SACTAttachment, name="Attachment List")
    active_attachment: bpy.props.IntProperty(name="Active Attachment", default=0, update=SACTSubtype.active_subtype_update)

    actions: bpy.props.CollectionProperty(type=SACTAction.SACTAction, name="Actor Action List")
    active_action: bpy.props.IntProperty(name="Active Actor Action", default=0, update=SACTAction.active_action_update)
    show_actions: bpy.props.BoolProperty()

# Regex RNA path matchers
scale_matcher = re.compile(r'pose.bones\["(\S+)"\].scale')
rotation_matcher = re.compile(r'pose.bones\["(\S+)"\].rotation')
location_matcher = re.compile(r'pose.bones\["(\S+)"\].location')

def write_action_channels(writebuf, action):
    # Set of frame indices
    frame_set = set()

    # Set of unique bone names
    bone_set = []

    # Scan through all fcurves to build animated bone set
    for fcurve in action.fcurves:
        data_path = fcurve.data_path
        scale_match = scale_matcher.match(data_path)
        rotation_match = rotation_matcher.match(data_path)
        location_match = location_matcher.match(data_path)

        if scale_match:
            if scale_match.group(1) not in bone_set:
                bone_set.append(scale_match.group(1))
        elif rotation_match:
            if rotation_match.group(1) not in bone_set:
                bone_set.append(rotation_match.group(1))
        elif location_match:
            if location_match.group(1) not in bone_set:
                bone_set.append(location_match.group(1))
        else:
            continue

        # Count unified keyframes for interleaving channel data
        for key in fcurve.keyframe_points:
            frame_set.add(int(key.co[0]))

    # Build bone table
    bone_list = []
    for bone in bone_set:
        fc_dict = dict()
        rotation_mode = None
        property_bits = 0
        for fcurve in action.fcurves:
            if fcurve.data_path == 'pose.bones["'+bone+'"].scale':
                if 'scale' not in fc_dict:
                    fc_dict['scale'] = [None, None, None]
                    property_bits |= 4
                fc_dict['scale'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_euler':
                if 'rotation_euler' not in fc_dict:
                    fc_dict['rotation_euler'] = [None, None, None]
                    rotation_mode = 'rotation_euler'
                    property_bits |= 1
                fc_dict['rotation_euler'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_quaternion':
                if 'rotation_quaternion' not in fc_dict:
                    fc_dict['rotation_quaternion'] = [None, None, None, None]
                    rotation_mode = 'rotation_quaternion'
                    property_bits |= 1
                fc_dict['rotation_quaternion'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_axis_angle':
                if 'rotation_axis_angle' not in fc_dict:
                    fc_dict['rotation_axis_angle'] = [None, None, None, None]
                    rotation_mode = 'rotation_axis_angle'
                    property_bits |= 1
                fc_dict['rotation_axis_angle'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].location':
                if 'location' not in fc_dict:
                    fc_dict['location'] = [None, None, None]
                    property_bits |= 2
                fc_dict['location'][fcurve.array_index] = fcurve
        bone_list.append((bone, rotation_mode, fc_dict, property_bits))

    # Write out frame indices
    sorted_frames = sorted(frame_set)
    writebuf(struct.pack('I', len(sorted_frames)))
    for frame in sorted_frames:
        writebuf(struct.pack('i', frame))

    # Interleave / interpolate keyframe data
    writebuf(struct.pack('I', len(bone_list)))
    for bone in bone_list:

        bone_name = bone[0]
        rotation_mode = bone[1]
        fc_dict = bone[2]
        property_bits = bone[3]

        writebuf(struct.pack('I', len(bone_name)))
        writebuf(bone_name.encode())

        writebuf(struct.pack('I', property_bits))

        writebuf(struct.pack('I', len(sorted_frames)))
        for frame in sorted_frames:

            # Rotation curves
            if rotation_mode == 'rotation_quaternion':
                quat = [0.0]*4
                for comp in range(4):
                    if fc_dict['rotation_quaternion'][comp]:
                        quat[comp] = fc_dict['rotation_quaternion'][comp].evaluate(frame)
                quat = Quaternion(quat).normalized()
                writebuf(struct.pack('ffff', quat[0], quat[1], quat[2], quat[3]))

            elif rotation_mode == 'rotation_euler':
                euler = [0.0]*3
                for comp in range(3):
                    if fc_dict['rotation_euler'][comp]:
                        euler[comp] = fc_dict['rotation_euler'][comp].evaluate(frame)
                euler_o = Euler(euler, 'XYZ')
                quat = euler_o.to_quaternion()
                writebuf(struct.pack('ffff', quat[0], quat[1], quat[2], quat[3]))

            elif rotation_mode == 'rotation_axis_angle':
                axis_angle = [0.0]*4
                for comp in range(4):
                    if fc_dict['rotation_axis_angle'][comp]:
                        axis_angle[comp] = fc_dict['rotation_axis_angle'][comp].evaluate(frame)
                quat = Quaternion(axis_angle[1:4], axis_angle[0])
                writebuf(struct.pack('ffff', quat[0], quat[1], quat[2], quat[3]))

            # Location curves
            if 'location' in fc_dict:
                writevec = [0.0]*3
                for comp in range(3):
                    if fc_dict['location'][comp]:
                        writevec[comp] = fc_dict['location'][comp].evaluate(frame)
                writebuf(struct.pack('fff', writevec[0], writevec[1], writevec[2]))

            # Scale curves
            if 'scale' in fc_dict:
                writevec = [1.0]*3
                for comp in range(3):
                    if fc_dict['scale'][comp]:
                        writevec[comp] = fc_dict['scale'][comp].evaluate(frame)
                writebuf(struct.pack('fff', writevec[0], writevec[1], writevec[2]))


def write_action_aabb(writebuf, arm_obj, mesh_obj, action):
    scene = bpy.context.scene

    # Mute root channels
    for fcurve in action.fcurves:
        fcurve.mute = fcurve.data_path == 'pose.bones["root"].location'

    # Transform against root
    root_bone = arm_obj.pose.bones['root']
    root_bone.location = (0.0,0.0,0.0)
    if root_bone.rotation_mode == 'QUATERNION':
        root_bone.rotation_quaternion = (1.0,0.0,0.0,0.0)
    else:
        root_bone.rotation_euler = (0.0,0.0,0.0)

    # Frame 1
    scene.frame_set(1)

    root_aabb_min = Vector(mesh_obj.bound_box[0])
    root_aabb_max = Vector(mesh_obj.bound_box[6])

    # Accumulate AABB for each frame
    for frame_idx in range(2, scene.frame_end + 1):
        scene.frame_set(frame_idx)

        test_aabb_min = Vector(mesh_obj.bound_box[0])
        test_aabb_max = Vector(mesh_obj.bound_box[6])

        for comp in range(3):
            if test_aabb_min[comp] < root_aabb_min[comp]:
                root_aabb_min[comp] = test_aabb_min[comp]
        for comp in range(3):
            if test_aabb_max[comp] > root_aabb_max[comp]:
                root_aabb_max[comp] = test_aabb_max[comp]

    # Unmute root channels
    for fcurve in action.fcurves:
        fcurve.mute = False

    writebuf(struct.pack('ffffff',
                         root_aabb_min[0], root_aabb_min[1], root_aabb_min[2],
                         root_aabb_max[0], root_aabb_max[1], root_aabb_max[2]))

def _out_armatures(sact_data, writebuf):
    writebuf(struct.pack('I', len(bpy.data.armatures)))
    for arm in bpy.data.armatures:
        writebuf(struct.pack('I', len(arm.name)))
        writebuf(arm.name.encode())

        if arm.library:
            arm_path = bpy.path.abspath(arm.library.filepath)
            writebuf(struct.pack('I', len(arm_path)))
            writebuf(arm_path.encode())
        else:
            writebuf(struct.pack('I', 0))

        armature.cook(writebuf, arm)

def _out_subtypes(sact_data, writebuf):
    writebuf(struct.pack('I', len(sact_data.subtypes)))
    for subtype in sact_data.subtypes:
        writebuf(struct.pack('I', len(subtype.name)))
        writebuf(subtype.name.encode())

        mesh = None
        if subtype.linked_mesh in bpy.data.objects:
            mesh = bpy.data.objects[subtype.linked_mesh]
            cskr_id = mesh.data.cskr_id
            writebuf(struct.pack('I', len(cskr_id)))
            writebuf(cskr_id.encode())
        else:
            writebuf(struct.pack('I', 0))

        if mesh and mesh.data.library:
            mesh_path = bpy.path.abspath(mesh.data.library.filepath)
            writebuf(struct.pack('I', len(mesh_path)))
            writebuf(mesh_path.encode())
        else:
            writebuf(struct.pack('I', 0))

        arm = None
        if subtype.linked_armature in bpy.data.objects:
            arm = bpy.data.objects[subtype.linked_armature]

        arm_idx = -1
        if arm:
            arm_idx = bpy.data.armatures.find(arm.name)
        writebuf(struct.pack('i', arm_idx))

        writebuf(struct.pack('I', len(subtype.overlays)))
        for overlay in subtype.overlays:
            writebuf(struct.pack('I', len(overlay.name)))
            writebuf(overlay.name.encode())

            mesh = None
            if overlay.linked_mesh in bpy.data.objects:
                mesh = bpy.data.objects[overlay.linked_mesh]
                cskr_id = mesh.data.cskr_id
                writebuf(struct.pack('I', len(cskr_id)))
                writebuf(cskr_id.encode())
            else:
                writebuf(struct.pack('I', 0))

            if mesh and mesh.data.library:
                mesh_path = bpy.path.abspath(mesh.data.library.filepath)
                writebuf(struct.pack('I', len(mesh_path)))
                writebuf(mesh_path.encode())
            else:
                writebuf(struct.pack('I', 0))

def _out_attachments(sact_data, writebuf):
    writebuf(struct.pack('I', len(sact_data.attachments)))
    for attachment in sact_data.attachments:
        writebuf(struct.pack('I', len(attachment.name)))
        writebuf(attachment.name.encode())

        mesh = None
        if attachment.linked_mesh in bpy.data.objects:
            mesh = bpy.data.objects[attachment.linked_mesh]
            cskr_id = mesh.data.cskr_id
            writebuf(struct.pack('I', len(cskr_id)))
            writebuf(cskr_id.encode())
        else:
            writebuf(struct.pack('I', 0))

        if mesh and mesh.data.library:
            mesh_path = bpy.path.abspath(mesh.data.library.filepath)
            writebuf(struct.pack('I', len(mesh_path)))
            writebuf(mesh_path.encode())
        else:
            writebuf(struct.pack('I', 0))

        arm = None
        if attachment.linked_armature in bpy.data.objects:
            arm = bpy.data.objects[attachment.linked_armature]

        arm_idx = -1
        if arm:
            arm_idx = bpy.data.armatures.find(arm.name)
        writebuf(struct.pack('i', arm_idx))

def _out_actions(sact_data, writebuf):
    writebuf(struct.pack('I', len(sact_data.actions)))
    for action_idx in range(len(sact_data.actions)):
        sact_data.active_action = action_idx
        action = sact_data.actions[action_idx]
        writebuf(struct.pack('I', len(action.name)))
        writebuf(action.name.encode())

        bact = None
        if action.name in bpy.data.actions:
            bact = bpy.data.actions[action.name]
            anim_id = bact.anim_id
            writebuf(struct.pack('I', len(anim_id)))
            writebuf(anim_id.encode())
        if not bact:
            raise RuntimeError('action %s not found' % action.name)

        writebuf(struct.pack('f', 1.0 / bact.hecl_fps))
        writebuf(struct.pack('b', int(bact.hecl_additive)))
        writebuf(struct.pack('b', int(bact.hecl_looping)))

        write_action_channels(writebuf, bact)
        writebuf(struct.pack('I', len(sact_data.subtypes)))
        for subtype_idx in range(len(sact_data.subtypes)):
            subtype = sact_data.subtypes[subtype_idx]
            sact_data.active_subtype = subtype_idx
            bpy.ops.scene.sactaction_load()
            if subtype.linked_armature not in bpy.data.objects:
                raise RuntimeError('armature %s not found' % subtype.linked_armature)
            arm = bpy.data.objects[subtype.linked_armature]
            if subtype.linked_mesh not in bpy.data.objects:
                raise RuntimeError('mesh %s not found' % subtype.linked_mesh)
            mesh = bpy.data.objects[subtype.linked_mesh]
            write_action_aabb(writebuf, arm, mesh, bact)

def _out_action_no_subtypes(sact_data, writebuf, action_name):
    for action_idx in range(len(sact_data.actions)):
        action = sact_data.actions[action_idx]
        if action.name == action_name:
            sact_data.active_action = action_idx
            writebuf(struct.pack('I', len(action.name)))
            writebuf(action.name.encode())

            bact = None
            if action.name in bpy.data.actions:
                bact = bpy.data.actions[action.name]
                anim_id = bact.anim_id
                writebuf(struct.pack('I', len(anim_id)))
                writebuf(anim_id.encode())
            if not bact:
                raise RuntimeError('action %s not found' % action.name)

            writebuf(struct.pack('f', 1.0 / bact.hecl_fps))
            writebuf(struct.pack('b', int(bact.hecl_additive)))
            writebuf(struct.pack('b', int(bact.hecl_looping)))

            write_action_channels(writebuf, bact)
            writebuf(struct.pack('I', 0))
            return

    raise RuntimeError("Unable to find action '%s'" % action_name)

# Cook
def cook(writebuf):
    bpy.context.scene.hecl_auto_remap = False
    sact_data = bpy.context.scene.hecl_sact_data

    # Output armatures
    _out_armatures(sact_data, writebuf)

    # Output subtypes
    _out_subtypes(sact_data, writebuf)

    # Output attachments
    _out_attachments(sact_data, writebuf)

    # Output actions
    _out_actions(sact_data, writebuf)

# Cook Character Data only
def cook_character_only(writebuf):
    sact_data = bpy.context.scene.hecl_sact_data

    # Output armatures
    _out_armatures(sact_data, writebuf)

    # Output subtypes
    _out_subtypes(sact_data, writebuf)

    # Output attachments
    _out_attachments(sact_data, writebuf)

    # Output no actions
    writebuf(struct.pack('I', 0))

def cook_action_channels_only(writebuf, action_name):
    sact_data = bpy.context.scene.hecl_sact_data

    # Output action without AABBs
    _out_action_no_subtypes(sact_data, writebuf, action_name)

# Access actor's contained subtype names
def get_subtype_names(writebuf):
    sact_data = bpy.context.scene.hecl_sact_data
    writebuf(struct.pack('I', len(sact_data.subtypes)))
    for sub_idx in range(len(sact_data.subtypes)):
        subtype = sact_data.subtypes[sub_idx]
        writebuf(struct.pack('I', len(subtype.name)))
        writebuf(subtype.name.encode())
        obj = bpy.data.objects[subtype.linked_mesh]
        cskr_id = obj.data.cskr_id
        writebuf(struct.pack('I', len(cskr_id)))
        writebuf(cskr_id.encode())

# Access subtype's contained overlay names
def get_subtype_overlay_names(writebuf, subtypeName):
    sact_data = bpy.context.scene.hecl_sact_data
    for sub_idx in range(len(sact_data.subtypes)):
        subtype = sact_data.subtypes[sub_idx]
        if subtype.name == subtypeName:
            writebuf(struct.pack('I', len(subtype.overlays)))
            for overlay in subtype.overlays:
                writebuf(struct.pack('I', len(overlay.name)))
                writebuf(overlay.name.encode())
                obj = bpy.data.objects[overlay.linked_mesh]
                cskr_id = obj.data.cskr_id
                writebuf(struct.pack('I', len(cskr_id)))
                writebuf(cskr_id.encode())
            return
    writebuf(struct.pack('I', 0))

# Access contained attachment names
def get_attachment_names(writebuf):
    sact_data = bpy.context.scene.hecl_sact_data
    writebuf(struct.pack('I', len(sact_data.attachments)))
    for att_idx in range(len(sact_data.attachments)):
        attachment = sact_data.attachments[att_idx]
        writebuf(struct.pack('I', len(attachment.name)))
        writebuf(attachment.name.encode())
        obj = bpy.data.objects[attachment.linked_mesh]
        cskr_id = obj.data.cskr_id
        writebuf(struct.pack('I', len(cskr_id)))
        writebuf(cskr_id.encode())

# Access actor's contained action names
def get_action_names(writebuf):
    sact_data = bpy.context.scene.hecl_sact_data
    writebuf(struct.pack('I', len(sact_data.actions)))
    for action_idx in range(len(sact_data.actions)):
        action = sact_data.actions[action_idx]
        writebuf(struct.pack('I', len(action.name)))
        writebuf(action.name.encode())
        anim_id = bpy.data.actions[action.name].anim_id
        writebuf(struct.pack('I', len(anim_id)))
        writebuf(anim_id.encode())

# Panel draw
def draw(layout, context):
    SACTSubtype.draw(layout.box(), context)
    SACTAction.draw(layout.box(), context)


# Time-remap option update
def time_remap_update(self, context):
    if context.scene.hecl_type == 'ACTOR':
        SACTAction.active_action_update(self, context)


# Registration
def register():
    SACTSubtype.register()
    SACTAction.register()
    bpy.utils.register_class(SACTData)
    bpy.types.Scene.hecl_sact_data = bpy.props.PointerProperty(type=SACTData)
    bpy.types.Action.anim_id = bpy.props.StringProperty(name='Original ANIM ID')
    bpy.types.Action.hecl_fps = bpy.props.IntProperty(name='HECL Action FPS', default=30)
    bpy.types.Action.hecl_additive = bpy.props.BoolProperty(name='HECL Additive Action', default=False)
    bpy.types.Action.hecl_looping = bpy.props.BoolProperty(name='HECL Looping Action', default=False)
    bpy.types.Scene.hecl_auto_remap = bpy.props.BoolProperty(name="Auto Remap",
        description="Enables automatic 60-fps time-remapping for playback-validation purposes",
        default=True, update=time_remap_update)

def unregister():
    bpy.utils.unregister_class(SACTData)
    SACTSubtype.unregister()
    SACTAction.unregister()
