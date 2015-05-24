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
from . import hmdl_shader
from . import hmdl_skin
from . import hmdl_mesh
from . import hmdl_anim

def get_3d_context(object_):
    window = bpy.context.window_manager.windows[0]
    screen = window.screen
    for area in screen.areas:
        if area.type == "VIEW_3D":
            area3d = area
            break
    for region in area3d.regions:
        if region.type == "WINDOW":
            region3d = region
            break
    override = {
        "window": window,
        "screen": screen,
        "area": area3d,
        "region": region3d,
        "object": object_
    }
    
    return override


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
def cook(writefd, platform_type, endian_char):
    mesh_obj = bpy.data.objects[bpy.context.scene.hecl_mesh_obj]
    if mesh_obj.type != 'MESH':
        raise RuntimeError("%s is not a mesh" % mesh_obj.name)

    # Partial meshes
    part_meshes = set()
    
    # Start with shader, mesh and rigging-info generation.
    # Use SHA1 hashing to determine what the ID-hash will be when
    # shaders are packaged; strip out duplicates
    shader_set = []
    rigger = None
    
    # Normalize all vertex weights
    override = get_3d_context(mesh_obj)
    try:
        bpy.ops.object.vertex_group_normalize_all(override, lock_active=False)
    except:
        pass
    
    # Copy mesh (and apply mesh modifiers)
    copy_name = mesh_obj.name + "_hmdltri"
    copy_mesh = bpy.data.meshes.new(copy_name)
    copy_obj = bpy.data.objects.new(copy_name, copy_mesh)
    copy_obj.data = mesh_obj.to_mesh(bpy.context.scene, True, 'RENDER')
    copy_obj.scale = mesh_obj.scale
    bpy.context.scene.objects.link(copy_obj)
    
    # If skinned, establish rigging generator
    if len(mesh_obj.vertex_groups):
        rigger = hmdl_skin.hmdl_skin(max_bone_count, mesh_obj.vertex_groups)

    # Determine count of transformation matricies to deliver to shader set
    actual_max_bone_counts = [1] * len(mesh_obj.data.materials)
    max_bone_count = 1
    for mat_idx in range(len(mesh_obj.data.materials)):
        mat = mesh_obj.data.materials[mat_idx]
        count = hmdl_shader.max_transform_counter(mat, mesh_obj)
        if count > 1:
            actual_max_bone_counts[mat_idx] = count
            if count > max_bone_count:
                max_bone_count = count

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
    actual_max_texmtx_count = 0
    for mat_idx in sorted_material_idxs:
        
        shader_hashes = []
        shader_uv_count = 0
        
        if mesh_obj.data.hecl_material_count > 0:
            for grp_idx in range(mesh_obj.data.hecl_material_count):
                for mat in bpy.data.materials:
                    if mat.name.endswith('_%u_%u' % (grp_idx, mat_idx)):
                        hecl_str = hmdl_shader.shader(mat, mesh_obj, bpy.data.filepath)
    
        else:
            mat = mesh_obj.data.materials[mat_idx]
            hecl_str = hmdl_shader.shader(mat, mesh_obj, bpy.data.filepath)

        mesh_maker = hmdl_mesh.hmdl_mesh()
        
        # Make special version of mesh with just the relevant material;
        # Also perform triangulation
        mesh = bpy.data.meshes.new(copy_obj.name + '_' + str(mat_idx))
        part_meshes.add(mesh)
        bm = bmesh.new()
        bm.from_mesh(copy_obj.data)
        to_remove = []
        shader_center = Vector((0,0,0))
        shader_center_count = 0
        for face in bm.faces:
            if face.material_index != mat_idx:
                to_remove.append(face)
            else:
                shader_center += face.calc_center_bounds()
                shader_center_count += 1
        shader_center /= shader_center_count
        bmesh.ops.delete(bm, geom=to_remove, context=5)
        bmesh.ops.triangulate(bm, faces=bm.faces)
        bm.to_mesh(mesh)
        bm.free()

        # Optimise mesh
        if rigger:
            mesh_maker.add_mesh(mesh, rigger, shader_uv_count)
        else:
            mesh_maker.add_mesh(mesh, None, shader_uv_count)

        shader_set.append((shader_hashes, mesh_maker, shader_center))

    # Filter out useless AABB points and generate data array
    aabb = bytearray()
    for comp in copy_obj.bound_box[0]:
        aabb += struct.pack(endian_char + 'f', comp)
    for comp in copy_obj.bound_box[6]:
        aabb += struct.pack(endian_char + 'f', comp)

    # Delete copied mesh from scene
    bpy.context.scene.objects.unlink(copy_obj)
    bpy.data.objects.remove(copy_obj)
    bpy.data.meshes.remove(copy_mesh)

    # Count total collections
    total_collections = 0
    for shader in shader_set:
        total_collections += len(shader[1].collections)

    # Start writing master buffer
    output_data = bytearray()
    output_data += aabb
    output_data += struct.pack(endian_char + 'III', mesh_obj.data.hecl_material_count, len(shader_set), total_collections)

    # Shader Reference Data (truncated SHA1 hashes)
    if mesh_obj.data.hecl_material_count > 0:
        for grp_idx in range(mesh_obj.data.hecl_material_count):
            for shader in shader_set:
                output_data += shader[0][grp_idx]
    else:
        for shader in shader_set:
            for subshader in shader[0]:
                output_data += subshader

    # Generate mesh data
    for shader in shader_set:
        mesh_maker = shader[1]
        output_data += struct.pack(endian_char + 'Ifff', len(mesh_maker.collections), shader[2][0], shader[2][1], shader[2][2])
        for coll_idx in range(len(mesh_maker.collections)):
        
            # Vert Buffer
            uv_count, max_bones, vert_bytes, vert_arr = mesh_maker.generate_vertex_buffer(coll_idx, endian_char)
            output_data += struct.pack(endian_char + 'III', uv_count, max_bones // 4, len(vert_bytes))
            output_data += vert_bytes

            # Elem Buffer
            collection_primitives, element_bytes, elem_arr = mesh_maker.generate_element_buffer(coll_idx, endian_char)
            output_data += struct.pack(endian_char + 'I', len(element_bytes))
            output_data += element_bytes

            # Index Buffer
            index_bytes = mesh_maker.generate_index_buffer(collection_primitives, endian_char, rigger)
            output_data += struct.pack(endian_char + 'I', len(index_bytes))
            output_data += index_bytes

    # Generate rigging data
    skin_info = None
    if rigger:
        skin_info = rigger.generate_rigging_info(endian_char)

    # Write final buffer
    final_data = bytearray()
    final_data = b'HMDL'
    if rigger:
        final_data += struct.pack(endian_char + 'IIII', 1, actual_max_texmtx_count, max_bone_count, len(skin_info))
        final_data += skin_info
    else:
        final_data += struct.pack(endian_char + 'II', 0, actual_max_texmtx_count)
    final_data += output_data

    # Clean up
    for mesh in part_meshes:
        bpy.data.meshes.remove(mesh)

    # Write final mesh object
    if area_db_id is not None:
        new_hash = 0
    else:
        new_hash = heclpak.add_object(final_data, b'HMDL', resource_name)
        res_db.update_resource_stats(db_id, new_hash)

    return db_id, new_hash, final_data


def panel_draw(self, context):
    layout = self.layout
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


# Debug register operators
import bpy
def register():
    bpy.types.Scene.hecl_mesh_obj = bpy.props.StringProperty(
        name='HECL Mesh Object',
        description='Blender Mesh Object to export during HECL\'s cook process')
    bpy.utils.register_class(hmdl_shader.hecl_shader_operator)
    pass
def unregister():
    bpy.utils.unregister_class(hmdl_shader.hecl_shader_operator)
    pass
