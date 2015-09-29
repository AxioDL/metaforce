from . import SACTSubtype, SACTAction, ANIM
from .. import hmdl

import bpy
import re
import os.path
import posixpath
import struct
from mathutils import Vector

# Actor data class
class SACTData(bpy.types.PropertyGroup):

    subtypes =\
    bpy.props.CollectionProperty(type=SACTSubtype.SACTSubtype, name="Actor Subtype List")
    active_subtype =\
    bpy.props.IntProperty(name="Active Actor Subtype", default=0, update=SACTSubtype.active_subtype_update)
    show_subtypes =\
    bpy.props.BoolProperty()

    actions =\
    bpy.props.CollectionProperty(type=SACTAction.SACTAction, name="Actor Action List")
    active_action =\
    bpy.props.IntProperty(name="Active Actor Action", default=0, update=SACTAction.active_action_update)
    show_actions =\
    bpy.props.BoolProperty()

    #show_events =\
    #bpy.props.BoolProperty()

# A Routine to resolve HECL DAG-relative paths while ensuring database path-constraints
def resolve_local_path(blend_path, rel_path):
    if not rel_path.startswith('//'):
        raise RuntimeError("Files must have relative paths")
    blend_dir = os.path.split(blend_path)[0]
    image_comps = re.split('/|\\\\', rel_path[2:])
    start_idx = 0
    for comp in image_comps:
        if comp == '..':
            start_idx += 1
            if not blend_dir:
                raise RuntimeError("Relative file path has exceeded DAG root")
            blend_dir = os.path.split(blend_dir)[0]
        else:
            break
    retval = blend_dir
    for i in range(len(image_comps)-start_idx):
        if retval:
            retval += '/'
        retval += image_comps[start_idx+i]
    return posixpath.relpath(retval)


# RARM Generator
def package_rarm(arm_obj, res_db, heclpak, arg_path, arg_package):

    rarm_db_id, rarm_hash = res_db.register_resource(arg_path, arm_obj.name, arg_package)
    if not rarm_hash:

        skeleton_data = hecl_rmdl.generate_skeleton_info(arm_obj)
        rarm_hash = heclpak.add_object(skeleton_data, b'RARM')
        res_db.update_resource_stats(rarm_db_id, rarm_hash)

    return rarm_db_id, rarm_hash

# RANI Generator
def package_rani(action_obj, res_db, heclpak, arg_path, arg_package):

    rani_db_id, rani_hash = res_db.register_resource(arg_path, action_obj.name, arg_package)
    if not rani_hash:

        res_db.clear_dependencies(rani_db_id)
        rani_hash = heclpak.add_object(hecl_rmdl.rmdl_anim.generate_animation_info(action_obj, res_db, rani_db_id, arg_package), b'RANI')
        res_db.update_resource_stats(rani_db_id, rani_hash)

    return rani_db_id, rani_hash


# Actor Ticket Generator
def package_actor(scene, res_db, heclpak, arg_path, arg_package, arg_res_name):
    actor_data = scene.hecl_sact_data

    act_db_id, act_hash = res_db.register_resource(arg_path, None, arg_package)
    res_db.clear_dependencies(act_db_id)

    with open(os.path.splitext(bpy.data.filepath)[0] + '.heclticket', 'wb') as ticket:

        # Subtypes
        ticket.write(struct.pack('I', len(actor_data.subtypes)))
        for subtype_idx in range(len(actor_data.subtypes)):
            subtype = actor_data.subtypes[subtype_idx]
            scene.hecl_sact_data.active_subtype = subtype_idx

            # Subtype name
            ticket.write(subtype.name.encode() + b'\0')

            # Mesh
            if subtype.linked_mesh in bpy.data.objects:
                mesh_obj = bpy.data.objects[subtype.linked_mesh]
                if mesh_obj.library:
                    path = resolve_local_path(arg_path.split(';')[-1], mesh_obj.library.filepath)
                    mesh_db_id, mesh_hash = res_db.search_for_resource(path, arg_package)
                    if not mesh_hash:
                        raise RuntimeError("Error - unable to load mesh library '{0}'".format(path))
                    res_db.register_dependency(act_db_id, mesh_db_id)
                    ticket.write(mesh_hash)
                else:
                    mesh_db_id, mesh_hash, _final_data = hecl_rmdl.to_rmdl(mesh_obj, mesh_obj.name, res_db, heclpak, arg_path, arg_package)
                    res_db.register_dependency(act_db_id, mesh_db_id)
                    ticket.write(mesh_hash)
            else:
                raise RuntimeError("Error - unable to load mesh '{0}'".format(mesh))

            # Armature
            if subtype.linked_armature in bpy.data.objects:
                arm_obj = bpy.data.objects[subtype.linked_armature]
                rarm_db_id, rarm_hash = package_rarm(arm_obj, res_db, heclpak, arg_path, arg_package)
                res_db.register_dependency(act_db_id, rarm_db_id)
                ticket.write(rarm_hash)
            else:
                raise RuntimeError("Error - unable to load armature '{0}'".format(subtype.linked_armature))

            # Action AABBs
            print('\nComputing Action AABBs for', subtype.name)
            scene.hecl_auto_remap = False
            ticket.write(struct.pack('I', len(actor_data.actions)))
            for action_idx in range(len(actor_data.actions)):
                action = actor_data.actions[action_idx]
                print(action.name)
                scene.hecl_sact_data.active_action = action_idx
                bpy.ops.scene.SACTAction_load()

                # Action name
                ticket.write(action.name.encode() + b'\0')

                # Frame 1
                scene.frame_set(1)

                # Transform against root
                root_bone = arm_obj.pose.bones['root']
                root_bone.location = (0.0,0.0,0.0)
                if root_bone.rotation_mode == 'QUATERNION':
                    root_bone.rotation_quaternion = (1.0,0.0,0.0,0.0)
                else:
                    root_bone.rotation_euler = (0.0,0.0,0.0)
                root_aabb_min = Vector(mesh_obj.bound_box[0])
                root_aabb_max = Vector(mesh_obj.bound_box[6])

                # Accumulate AABB for each frame
                for frame_idx in range(2, scene.frame_end + 1):
                    scene.frame_set(frame_idx)

                    root_bone.location = (0.0,0.0,0.0)
                    scene.update()
                    if root_bone.rotation_mode == 'QUATERNION':
                        root_bone.rotation_quaternion = (1.0,0.0,0.0,0.0)
                    else:
                        root_bone.rotation_euler = (0.0,0.0,0.0)
                    test_aabb_min = Vector(mesh_obj.bound_box[0])
                    test_aabb_max = Vector(mesh_obj.bound_box[6])

                    for comp in range(3):
                        if test_aabb_min[comp] < root_aabb_min[comp]:
                            root_aabb_min[comp] = test_aabb_min[comp]
                    for comp in range(3):
                        if test_aabb_max[comp] > root_aabb_max[comp]:
                            root_aabb_max[comp] = test_aabb_max[comp]

                ticket.write(struct.pack('ffffff',
                                         root_aabb_min[0], root_aabb_min[1], root_aabb_min[2],
                                         root_aabb_max[0], root_aabb_max[1], root_aabb_max[2]))


        # Actions
        anim_hashes = dict()
        ticket.write(struct.pack('I', len(actor_data.actions)))
        for action in actor_data.actions:

            # Action name
            ticket.write(action.name.encode() + b'\0')

            if action.type == 'SINGLE':
                ticket.write(struct.pack('I', 0))
                action_name = action.subactions[0].name
                if action_name not in bpy.data.actions:
                    raise RuntimeError("Error - unable to load action '{0}'".format(action_name))
                if action_name in anim_hashes:
                    rani_hash = anim_hashes[action_name]
                else:
                    action_obj = bpy.data.actions[action_name]
                    rani_db_id, rani_hash = package_rani(action_obj, res_db, heclpak, arg_path, arg_package)
                    res_db.register_dependency(act_db_id, rani_db_id)
                    anim_hashes[action_name] = rani_hash
                ticket.write(rani_hash)

            elif action.type == 'SEQUENCE':
                ticket.write(struct.pack('II', 1, len(action.subactions)))
                for subaction in action.subactions:
                    action_name = subaction.name
                    if action_name not in bpy.data.actions:
                        raise RuntimeError("Error - unable to load action '{0}'".format(action_name))
                    if action_name in anim_hashes:
                        rani_hash = anim_hashes[action_name]
                    else:
                        action_obj = bpy.data.actions[action_name]
                        rani_db_id, rani_hash = package_rani(action_obj,
                                                             res_db, heclpak, arg_path, arg_package)
                        res_db.register_dependency(act_db_id, rani_db_id)
                        anim_hashes[action_name] = rani_hash
                    ticket.write(rani_hash)

            elif action.type == 'RANDOM':
                ticket.write(struct.pack('III', 2, len(action.subactions), action.random_val))
                for subaction in action.subactions:
                    action_name = subaction.name
                    if action_name not in bpy.data.actions:
                        raise RuntimeError("Error - unable to load action '{0}'".format(action_name))
                    if action_name in anim_hashes:
                        rani_hash = anim_hashes[action_name]
                    else:
                        action_obj = bpy.data.actions[action_name]
                        rani_db_id, rani_hash = package_rani(action_obj,
                                                             res_db, heclpak, arg_path, arg_package)
                        res_db.register_dependency(act_db_id, rani_db_id)
                        anim_hashes[action_name] = rani_hash
                    ticket.write(rani_hash)


# Cook
def cook(writebuffunc, platform, endianchar):
    print('COOKING SACT')


# Panel draw
def draw(layout, context):
    SACTSubtype.draw(layout.box(), context)
    SACTAction.draw(layout.box(), context)
    #SACTEvent.draw(layout.box(), context)


# Time-remap option update
def time_remap_update(self, context):
    if context.scene.hecl_type == 'ACTOR':
        SACTAction.active_action_update(self, context)


# Registration
def register():
    SACTSubtype.register()
    #SACTEvent.register()
    SACTAction.register()
    bpy.utils.register_class(SACTData)
    bpy.types.Scene.hecl_sact_data = bpy.props.PointerProperty(type=SACTData)
    bpy.types.Action.hecl_fps = bpy.props.IntProperty(name='HECL Action FPS', default=30)
    bpy.types.Action.hecl_additive = bpy.props.BoolProperty(name='HECL Additive Action', default=False)
    bpy.types.Scene.hecl_auto_remap = bpy.props.BoolProperty(name="Auto Remap",
        description="Enables automatic 60-fps time-remapping for playback-validation purposes",
        default=True, update=time_remap_update)

def unregister():
    bpy.utils.unregister_class(SACTData)
    SACTSubtype.unregister()
    SACTAction.unregister()
    #SACTEvent.unregister()
