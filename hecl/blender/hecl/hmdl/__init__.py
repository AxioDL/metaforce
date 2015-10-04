'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>

This Python module provides a generator implementation for
the 'HMDL' mesh format designed for use with HECL.

The format features three main sections:
* Shader table
* Skin-binding table
* Mesh table (VBOs [array,element], VAO attribs, drawing index)

The Shader table provides index-referenced binding points
for mesh-portions to use for rendering. 

The Skin-binding table provides the runtime with identifiers
to use in ensuring the correct bone-transformations are bound
to the shader when rendering a specific primitive.

The Mesh table contains Vertex and Element buffers with interleaved
Positions, Normals, UV coordinates, and Weight Vectors
'''

import struct, bpy, bmesh
from mathutils import Vector
from . import HMDLShader, HMDLSkin, HMDLMesh

# Generate Skeleton Info structure (free-form tree structure)
def generate_skeleton_info(armature, endian_char='<'):
    
    bones = []
    for bone in armature.data.bones:
        bone_bytes = bytearray()
        
        # Write bone hash
        bone_bytes += struct.pack(endian_char + 'I', hmdl_anim.hashbone(bone.name))
        
        for comp in bone.head_local:
            bone_bytes += struct.pack(endian_char + 'f', comp)

        parent_idx = -1
        if bone.parent:
            parent_idx = armature.data.bones.find(bone.parent.name)
        bone_bytes += struct.pack(endian_char + 'i', parent_idx)

        bone_bytes += struct.pack(endian_char + 'I', len(bone.children))

        for child in bone.children:
            child_idx = armature.data.bones.find(child.name)
            bone_bytes += struct.pack(endian_char + 'I', child_idx)
                
        bones.append(bone_bytes)

    # Generate bone tree data
    info_bytes = bytearray()
    info_bytes += struct.pack(endian_char + 'I', len(bones))
    
    cur_offset = len(bones) * 4 + 4
    for bone in bones:
        info_bytes += struct.pack(endian_char + 'I', cur_offset)
        cur_offset += len(bone)
            
    for bone in bones:
        info_bytes += bone
            
    return info_bytes


# Takes a Blender 'Mesh' object (not the datablock)
# and performs a one-shot conversion process to HMDL; packaging
# into the HECL data-pipeline and returning a hash once complete
def cook(writebuffunc, mesh_obj, max_skin_banks):
    if mesh_obj.type != 'MESH':
        raise RuntimeError("%s is not a mesh" % mesh_obj.name)
    
    # Copy mesh (and apply mesh modifiers)
    copy_name = mesh_obj.name + "_hmdltri"
    copy_mesh = bpy.data.meshes.new(copy_name)
    copy_obj = bpy.data.objects.new(copy_name, copy_mesh)
    copy_obj.data = mesh_obj.to_mesh(bpy.context.scene, True, 'RENDER')
    copy_obj.scale = mesh_obj.scale
    bpy.context.scene.objects.link(copy_obj)

    # Create master triangulated BMesh and VertPool
    bm_master = bmesh.new()
    bm_master.from_mesh(copy_obj.data)
    bmesh.ops.triangulate(bm_master, faces=bm_master.faces)
    vert_pool = HMDLMesh.VertPool(bm_master)

    # Sort materials by pass index first
    sorted_material_idxs = []
    source_mat_set = set(range(len(mesh_obj.data.materials)))
    while len(source_mat_set):
        min_mat_idx = source_mat_set.pop()
        source_mat_set.add(min_mat_idx)
        for mat_idx in source_mat_set:
            if mesh_obj.data.materials[mat_idx].pass_index < mesh_obj.data.materials[min_mat_idx].pass_index:
                min_mat_idx = mat_idx
        sorted_material_idxs.append(min_mat_idx)
        source_mat_set.discard(min_mat_idx)

    # Generate shaders
    if mesh_obj.data.hecl_material_count > 0:
        writebuffunc(struct.pack('I', mesh_obj.data.hecl_material_count))
        for grp_idx in range(mesh_obj.data.hecl_material_count):
            writebuffunc(struct.pack('I', len(sorted_material_idxs)))
            for mat_idx in sorted_material_idxs:
                found = False
                for mat in bpy.data.materials:
                    if mat.name.endswith('_%u_%u' % (grp_idx, mat_idx)):
                        hecl_str, texs = HMDLShader.shader(mat, mesh_obj)
                        writebuffunc((hecl_str + '\n').encode())
                        writebuffunc(struct.pack('I', len(texs)))
                        for tex in texs:
                            writebuffunc((tex + '\n').encode())
                        found = True
                        break
                if not found:
                    raise RuntimeError('uneven material set %d in %s' % (grp_idx, mesh_obj.name))
    else:
        writebuffunc(struct.pack('II', 1, len(sorted_material_idxs)))
        for mat_idx in sorted_material_idxs:
            mat = mesh_obj.data.materials[mat_idx]
            hecl_str, texs = HMDLShader.shader(mat, mesh_obj)
            writebuffunc((hecl_str + '\n').encode())
            writebuffunc(struct.pack('I', len(texs)))
            for tex in texs:
                writebuffunc((tex + '\n').encode())

    # Output vert pool
    vert_pool.write_out(writebuffunc, mesh_obj.vertex_groups)

    dlay = None
    if len(bm_master.verts.layers.deform):
        dlay = bm_master.verts.layers.deform[0]

    # Generate island meshes
    for mat_idx in sorted_material_idxs:
        mat_faces_rem = []
        for face in bm_master.faces:
            if face.material_index == mat_idx:
                mat_faces_rem.append(face)
        while len(mat_faces_rem):
            the_list = []
            skin_slot_set = set()
            HMDLMesh.recursive_faces_islands(dlay, the_list, mat_faces_rem, skin_slot_set,
                                             max_skin_banks, mat_faces_rem[0])
            writebuffunc(struct.pack('B', 1))
            HMDLMesh.write_out_surface(writebuffunc, vert_pool, the_list, mat_idx)

    # No more surfaces
    writebuffunc(struct.pack('B', 0))

    # Filter out useless AABB points and generate data array
    #aabb = bytearray()
    #for comp in copy_obj.bound_box[0]:
    #    aabb += struct.pack('f', comp)
    #for comp in copy_obj.bound_box[6]:
    #    aabb += struct.pack('f', comp)

    # Delete copied mesh from scene
    bm_master.free()
    bpy.context.scene.objects.unlink(copy_obj)
    bpy.data.objects.remove(copy_obj)
    bpy.data.meshes.remove(copy_mesh)


def draw(layout, context):
    layout.prop_search(context.scene, 'hecl_mesh_obj', context.scene, 'objects')
    if not len(context.scene.hecl_mesh_obj):
        layout.label("Mesh not specified", icon='ERROR')
    elif context.scene.hecl_mesh_obj not in context.scene.objects:
        layout.label("'"+context.scene.hecl_mesh_obj+"' not in scene", icon='ERROR')
    else:
        obj = context.scene.objects[context.scene.hecl_mesh_obj]
        if obj.type != 'MESH':
            layout.label("'"+context.scene.hecl_mesh_obj+"' not a 'MESH'", icon='ERROR')
        layout.prop(obj.data, 'hecl_active_material')
        layout.prop(obj.data, 'hecl_material_count')

# Material update
def material_update(self, context):
    target_idx = self.hecl_active_material
    if target_idx >= self.hecl_material_count or target_idx < 0:
        return
    slot_count = len(self.materials)
    for mat_idx in range(slot_count):
        for mat in bpy.data.materials:
            if mat.name.endswith('_%u_%u' % (target_idx, mat_idx)):
                self.materials[mat_idx] = mat

import bpy
def register():
    bpy.types.Scene.hecl_mesh_obj = bpy.props.StringProperty(
        name='HECL Mesh Object',
        description='Blender Mesh Object to export during HECL\'s cook process')
    bpy.types.Scene.hecl_actor_obj = bpy.props.StringProperty(
        name='HECL Actor Object',
        description='Blender Empty Object to export during HECL\'s cook process')
    bpy.types.Mesh.hecl_material_count = bpy.props.IntProperty(name='HECL Material Count', default=0, min=0)
    bpy.types.Mesh.hecl_active_material = bpy.props.IntProperty(name='HECL Active Material', default=0, min=0, update=material_update)
    bpy.utils.register_class(HMDLShader.hecl_shader_operator)
    pass
def unregister():
    bpy.utils.unregister_class(HMDLShader.hecl_shader_operator)
    pass
