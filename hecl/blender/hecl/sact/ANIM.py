'''
This file provides a means to encode animation key-channels
in an interleaved, sparse array for use by the runtime
'''

import re
import struct
import mathutils

# Regex RNA path matchers
scale_matcher = re.compile(r'pose.bones\["(\S+)"\].scale')
rotation_matcher = re.compile(r'pose.bones\["(\S+)"\].rotation')
location_matcher = re.compile(r'pose.bones\["(\S+)"\].location')

# Effect transform modes
EFFECT_XF_MODES = {'STATIONARY':0, 'WORLD':1, 'LOCAL':2}

# Generate animation info
def generate_animation_info(action, res_db, rani_db_id, arg_package, endian_char='<'):
    
    # Set of frame indices
    frame_set = set()
    
    # Set of unique bone names
    bone_set = set()
    
    # Scan through all fcurves to build animated bone set
    for fcurve in action.fcurves:
        data_path = fcurve.data_path
        scale_match = scale_matcher.match(data_path)
        rotation_match = rotation_matcher.match(data_path)
        location_match = location_matcher.match(data_path)

        if scale_match:
            bone_set.add(scale_match.group(1))
        elif rotation_match:
            bone_set.add(rotation_match.group(1))
        elif location_match:
            bone_set.add(location_match.group(1))
        else:
            continue

        # Count unified keyframes for interleaving channel data
        for key in fcurve.keyframe_points:
            frame_set.add(int(key.co[0]))

    # Relate fcurves per-frame / per-bone and assemble data
    key_stream = bytearray()
    key_stream += struct.pack(endian_char + 'II', len(frame_set), len(bone_set))
    duration = action.frame_range[1] / action.hecl_fps
    interval = 1.0 / action.hecl_fps
    key_stream += struct.pack(endian_char + 'ff', duration, interval)
    
    # Generate keyframe bitmap
    fr = int(round(action.frame_range[1]))
    key_stream += struct.pack(endian_char + 'I', fr)
    bitmap_words = [0] * (fr // 32)
    if fr % 32:
        bitmap_words.append(0)
    for i in range(fr):
        if i in frame_set:
            bitmap_words[i//32] |= 1 << i%32
    for word in bitmap_words:
        key_stream += struct.pack(endian_char + 'I', word)


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
                    property_bits |= 1
                fc_dict['scale'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_euler':
                if 'rotation_euler' not in fc_dict:
                    fc_dict['rotation_euler'] = [None, None, None]
                    rotation_mode = 'rotation_euler'
                    property_bits |= 2
                fc_dict['rotation_euler'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_quaternion':
                if 'rotation_quaternion' not in fc_dict:
                    fc_dict['rotation_quaternion'] = [None, None, None, None]
                    rotation_mode = 'rotation_quaternion'
                    property_bits |= 2
                fc_dict['rotation_quaternion'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].rotation_axis_angle':
                if 'rotation_axis_angle' not in fc_dict:
                    fc_dict['rotation_axis_angle'] = [None, None, None, None]
                    rotation_mode = 'rotation_axis_angle'
                    property_bits |= 2
                fc_dict['rotation_axis_angle'][fcurve.array_index] = fcurve
            elif fcurve.data_path == 'pose.bones["'+bone+'"].location':
                if 'location' not in fc_dict:
                    fc_dict['location'] = [None, None, None]
                    property_bits |= 4
                fc_dict['location'][fcurve.array_index] = fcurve
        bone_list.append((bone, rotation_mode, fc_dict))
        bone_head = hashbone(bone)
        bone_head |= (property_bits << 28)
        key_stream += struct.pack(endian_char + 'I', bone_head)

    # Interleave / interpolate keyframe data
    for frame in sorted(frame_set):
        for bone in bone_list:
            
            bone_name = bone[0]
            rotation_mode = bone[1]
            fc_dict = bone[2]
            
            # Scale curves
            if 'scale' in fc_dict:
                for comp in range(3):
                    if fc_dict['scale'][comp]:
                        key_stream += struct.pack(endian_char + 'f', fc_dict['scale'][comp].evaluate(frame))
                    else:
                        key_stream += struct.pack(endian_char + 'f', 0.0)

            # Rotation curves
            if rotation_mode == 'rotation_quaternion':
                for comp in range(4):
                    if fc_dict['rotation_quaternion'][comp]:
                        key_stream += struct.pack(endian_char + 'f', fc_dict['rotation_quaternion'][comp].evaluate(frame))
                    else:
                        key_stream += struct.pack(endian_char + 'f', 0.0)

            elif rotation_mode == 'rotation_euler':
                euler = [0.0, 0.0, 0.0]
                for comp in range(3):
                    if fc_dict['rotation_euler'][comp]:
                        euler[comp] = fc_dict['rotation_euler'][comp].evaluate(frame)
                euler_o = mathutils.Euler(euler, 'XYZ')
                quat = euler_o.to_quaternion()
                key_stream += struct.pack(endian_char + 'ffff', quat[0], quat[1], quat[2], quat[3])

            elif rotation_mode == 'rotation_axis_angle':
                axis_angle = [0.0, 0.0, 0.0, 0.0]
                for comp in range(4):
                    if fc_dict['rotation_axis_angle'][comp]:
                        axis_angle[comp] = fc_dict['rotation_axis_angle'][comp].evaluate(frame)
                quat = mathutils.Quaternion(axis_angle[1:4], axis_angle[0])
                key_stream += struct.pack(endian_char + 'ffff', quat[0], quat[1], quat[2], quat[3])

            # Location curves
            if 'location' in fc_dict:
                for comp in range(3):
                    if fc_dict['location'][comp]:
                        key_stream += struct.pack(endian_char + 'f', fc_dict['location'][comp].evaluate(frame))
                    else:
                        key_stream += struct.pack(endian_char + 'f', 0.0)


    # Generate event buffer
    event_buf = bytearray()
    if hasattr(action, 'hecl_events'):
        c1 = 0
        c2 = 0
        c3 = 0
        c4 = 0
        for event in action.hecl_events:
            if event.type == 'LOOP':
                c1 += 1
            elif event.type == 'UEVT':
                c2 += 1
            elif event.type == 'EFFECT':
                c3 += 1
            elif event.type == 'SOUND':
                c4 += 1
        event_buf += struct.pack(endian_char + 'IIII', c1, c2, c3, c4)

        for event in action.hecl_events:
            if event.type == 'LOOP':
                event_buf += struct.pack(endian_char + 'fi', event.time, event.loop_data.bool)
        
        for event in action.hecl_events:
            if event.type == 'UEVT':
                event_buf += struct.pack(endian_char + 'fii', event.time, event.uevt_data.type,
                                         hashbone(event.uevt_data.bone_name))

        for event in action.hecl_events:
            if event.type == 'EFFECT':
                effect_db_id, effect_hash = res_db.search_for_resource(event.effect_data.uid, arg_package)
                if effect_hash:
                    res_db.register_dependency(rani_db_id, effect_db_id)
                else:
                    raise RuntimeError("Error - unable to find effect '{0}'".format(event.effect_data.uid))
                event_buf += struct.pack(endian_char + 'fiifi', event.time, event.effect_data.frame_count,
                                         hashbone(event.effect_data.bone_name), event.effect_data.scale,
                                         EFFECT_XF_MODES[event.effect_data.transform_mode])
                event_buf += effect_hash

        for event in action.hecl_events:
            if event.type == 'SOUND':
                sid = int.from_bytes(event.sound_data.sound_id.encode()[:4], byteorder='big', signed=False)
                event_buf += struct.pack(endian_char + 'fIff', event.time, sid,
                                         event.sound_data.ref_amp, event.sound_data.ref_dist)

    else:
        event_buf += struct.pack('IIII',0,0,0,0)



    return key_stream + event_buf

