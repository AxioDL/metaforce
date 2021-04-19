import struct

def cook(writebuf, arm):
    writebuf(struct.pack('I', len(arm.bones)))
    for bone in arm.bones:
        writebuf(struct.pack('I', len(bone.name)))
        writebuf(bone.name.encode())

        writebuf(struct.pack('fff', bone.head_local[0], bone.head_local[1], bone.head_local[2]))

        if bone.parent:
            writebuf(struct.pack('i', arm.bones.find(bone.parent.name)))
        else:
            writebuf(struct.pack('i', -1))

        writebuf(struct.pack('I', len(bone.children)))
        for child in bone.children:
            writebuf(struct.pack('i', arm.bones.find(child.name)))

def draw(layout, context):
    layout.prop_search(context.scene, 'hecl_arm_obj', context.scene, 'objects')
    if not len(context.scene.hecl_arm_obj):
        layout.label(text="Armature not specified", icon='ERROR')
    elif context.scene.hecl_arm_obj not in context.scene.objects:
        layout.label(text="'"+context.scene.hecl_arm_obj+"' not in scene", icon='ERROR')
    else:
        obj = context.scene.objects[context.scene.hecl_arm_obj]
        if obj.type != 'ARMATURE':
            layout.label(text="'"+context.scene.hecl_arm_obj+"' not an 'ARMATURE'", icon='ERROR')

import bpy
def register():
    bpy.types.Scene.hecl_arm_obj = bpy.props.StringProperty(
        name='HECL Armature Object',
        description='Blender Armature Object to export during HECL\'s cook process')
