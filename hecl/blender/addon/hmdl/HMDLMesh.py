'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>
'''

import struct
import bpy

class loop_vert:

    def __init__(self, mesh, loop):
        self.mesh = mesh
        self.loop = loop

    def __hash__(self):
        return (self.mesh, self.loop.index).__hash__()
    
    def __eq__(self, other):
        return self.mesh == other.mesh and self.loop.index == other.loop.index
    
    def __ne__(self, other):
        return self.mesh != other.mesh or self.loop.index != other.loop.index

    def __str__(self):
        return (self.mesh, self.loop.index).__str__()


# Round up to nearest 32 multiple
def ROUND_UP_32(num):
    return (num + 31) & ~31

# Round up to nearest 4 multiple
def ROUND_UP_4(num):
    return (num + 3) & ~3

# This routine conditionally inserts a loop into a multi-tiered
# array/set collection; simultaneously relating verts to loops and
# eliminating redundant loops (containing identical UV coordinates)
def _augment_loop_vert_array(lv_array, mesh, loop, uv_count):
    
    # Create loop_vert object for comparitive testing
    lv = loop_vert(mesh, loop)
    
    # First perform quick check to see if loop is already in a set
    for existing_loop_set in lv_array:
        if lv in existing_loop_set:
            return

    # Now perform extended check to see if any UV coordinate values already match
    for existing_loop_set in lv_array:
        for existing_loop in existing_loop_set:
            matches = True
            for uv_layer_idx in range(uv_count):
                uv_layer = mesh.uv_layers[uv_layer_idx]
                existing_uv_coords = uv_layer.data[existing_loop.loop.index].uv
                check_uv_coords = uv_layer.data[loop.index].uv
                if (existing_uv_coords[0] != check_uv_coords[0] or
                    existing_uv_coords[1] != check_uv_coords[1]):
                    matches = False
                    break
            if matches:
                existing_loop_set.append(lv)
                return

    # If we get here, no match found; add new set to `lv_array`
    lv_array.append([lv])


# Get loop set from collection generated with above method;
# containing a specified loop
def _get_loop_set(lv_array, mesh, loop):
    
    # Create loop_vert object for comparitive testing
    lv = loop_vert(mesh, loop)

    for existing_loop_set in lv_array:
        if lv in existing_loop_set:
            return existing_loop_set
    return None


# Method to find triangle opposite another triangle over two vert-indices
def _find_polygon_opposite_idxs(mesh, original_triangle, a_idx, b_idx):
    
    for triangle in mesh.polygons:
        
        if triangle == original_triangle:
            continue
        
        if (a_idx in triangle.vertices and b_idx in triangle.vertices):
            return triangle

    return None

# Method to find triangle opposite another triangle over two loop-vert sets
def _find_polygon_opposite_lvs(mesh, original_triangle, lv_a, lv_b):
    a_idx = lv_a[0].loop.vertex_index
    b_idx = lv_b[0].loop.vertex_index
    return _find_polygon_opposite_idxs(mesh, original_triangle, a_idx, b_idx)




class hmdl_mesh:
    
    def __init__(self):
        
        # 4-byte ID string used in generated HMDL file
        self.file_identifier = '_GEN'
        
        # Array that holds collections. A collection is a 16-bit index
        # worth of vertices, elements referencing them, and a
        # primitive array to draw them
        self.collections = []
    
    
    
    # If vertex index space is exceeded for a single additional vertex,
    # a new collection is created and returned by this routine
    def _check_collection_overflow(self, mesh, collection, rigger, uv_count):
        max_bone_count = 0;
        if rigger:
            max_bone_count = rigger.max_bone_count
        if not collection or len(collection['vertices']) >= 65535:
            new_collection = {'uv_count':uv_count, 'max_bone_count':max_bone_count, 'vertices':[], 'vert_weights':[], 'tri_strips':[]}
            self.collections.append(new_collection)
            return new_collection, True
        else:
            return collection, False


    # Augments draw generator with a single blender MESH data object
    def add_mesh(self, mesh, rigger, uv_count):
        max_bone_count = 0;
        if rigger:
            max_bone_count = rigger.max_bone_count

        print("Optimizing mesh:", mesh.name)
        opt_gpu_vert_count = 0
    
        # First, generate compressed loop-vertex array-array-set collection
        loop_vert_array = []
        for vert in mesh.vertices:
            loop_verts = []
            for loop in mesh.loops:
                if loop.vertex_index == vert.index:
                    _augment_loop_vert_array(loop_verts, mesh, loop, uv_count)
            loop_vert_array.append(loop_verts)
    

        # Find best collection to add mesh data into
        best_collection = None
        for collection in self.collections:
            if (collection['uv_count'] == uv_count and
                collection['max_bone_count'] == max_bone_count and
                len(collection['vertices']) < 65000):
                best_collection = collection
                break
        if not best_collection:
            # Create a new one if no good one found
            best_collection, is_new_collection = self._check_collection_overflow(mesh, None, rigger, uv_count)
    
        # If rigging, start an array of bone names to be bound to contiguous tri-strips
        tri_strip_bones = []
        tri_strip_bones_overflow = False
    
        # Now begin generating draw primitives
        visited_polys = set()
        for poly in mesh.polygons:
            # Skip if already visited
            if poly in visited_polys:
                continue
            
            # Allows restart if initial polygon was not added
            good = False
            while not good:
                
                # Begin a tri-strip primitive (array of vert indices)
                tri_strip = []
                
                # Temporary references to trace out strips of triangles
                temp_poly = poly
                
                # Rolling references of last two emitted loop-vert sets (b is older)
                last_loop_vert_a = None
                last_loop_vert_b = None
                    
                # In the event of vertex-buffer overflow, this will be made true;
                # resulting in the immediate splitting of a tri-strip
                is_new_collection = False
                

                
                # As long as there is a connected polygon to visit
                while temp_poly:
                    if 0 == len(tri_strip): # First triangle in strip
                        
                        # Order the loops so the last two connect to a next polygon
                        idx0 = mesh.loops[temp_poly.loop_indices[0]].vertex_index
                        idx1 = mesh.loops[temp_poly.loop_indices[1]].vertex_index
                        idx2 = mesh.loops[temp_poly.loop_indices[2]].vertex_index
                        if not _find_polygon_opposite_idxs(mesh, temp_poly, idx1, idx2):
                            loop_idxs = [temp_poly.loop_indices[2], temp_poly.loop_indices[0], temp_poly.loop_indices[1]]
                            if not _find_polygon_opposite_idxs(mesh, temp_poly, idx0, idx1):
                                loop_idxs = [temp_poly.loop_indices[1], temp_poly.loop_indices[2], temp_poly.loop_indices[0]]
                        else:
                            loop_idxs = temp_poly.loop_indices
                        
                        # Add three loop-vert vertices to tri-strip
                        for poly_loop_idx in loop_idxs:
                            poly_loop = mesh.loops[poly_loop_idx]
                            loop_vert = _get_loop_set(loop_vert_array[poly_loop.vertex_index], mesh, poly_loop)
                            
                            # If rigging, ensure that necessary bones are available and get weights
                            weights = None
                            if rigger:
                                weights = rigger.augment_bone_array_with_lv(mesh, tri_strip_bones, loop_vert)
                                if weights is None:
                                    tri_strip_bones_overflow = True
                                    break
                            
                            if loop_vert not in best_collection['vertices']:
                                best_collection, is_new_collection = self._check_collection_overflow(mesh, best_collection, rigger, uv_count)
                                if is_new_collection:
                                    break
                                best_collection['vertices'].append(loop_vert)
                                best_collection['vert_weights'].append(weights)
                            tri_strip.append(best_collection['vertices'].index(loop_vert))
                            last_loop_vert_b = last_loop_vert_a
                            last_loop_vert_a = loop_vert
                            opt_gpu_vert_count += 1
                            #print('appended initial loop', loop_vert[0].loop.index)

                        if is_new_collection or tri_strip_bones_overflow:
                            break
                
                    
                    else: # Not the first triangle in strip; look up all three loop-verts,
                          # ensure it matches last-2 rolling reference, emit remaining loop-vert
                        
                        # Iterate loop verts
                        odd_loop_vert_out = None
                        loop_vert_match_count = 0
                        for poly_loop_idx in temp_poly.loop_indices:
                            poly_loop = mesh.loops[poly_loop_idx]
                            loop_vert = _get_loop_set(loop_vert_array[poly_loop.vertex_index], mesh, poly_loop)
                        
                            if (loop_vert == last_loop_vert_a or loop_vert == last_loop_vert_b):
                                loop_vert_match_count += 1
                                continue
                    
                            odd_loop_vert_out = loop_vert
                        
                        
                        # Ensure there are two existing matches to continue tri-strip
                        if loop_vert_match_count != 2 or not odd_loop_vert_out:
                            break
                        
                        
                        # If rigging, ensure that necessary bones are available and get weights
                        weights = None
                        if rigger:
                            weights = rigger.augment_bone_array_with_lv(mesh, tri_strip_bones, odd_loop_vert_out)
                            if weights is None:
                                tri_strip_bones_overflow = True
                                break
                        
                        
                        # Add to tri-strip
                        if odd_loop_vert_out not in best_collection['vertices']:
                            best_collection, is_new_collection = self._check_collection_overflow(mesh, best_collection, rigger, uv_count)
                            if is_new_collection:
                                break
                            best_collection['vertices'].append(odd_loop_vert_out)
                            best_collection['vert_weights'].append(weights)
                        tri_strip.append(best_collection['vertices'].index(odd_loop_vert_out))
                        last_loop_vert_b = last_loop_vert_a
                        last_loop_vert_a = odd_loop_vert_out
                        opt_gpu_vert_count += 1



                    # This polygon is good
                    visited_polys.add(temp_poly)
                    
                    
                    # Find a polygon directly connected to this one to continue strip
                    temp_poly = _find_polygon_opposite_lvs(mesh, temp_poly, last_loop_vert_a, last_loop_vert_b)
                    if temp_poly in visited_polys:
                        temp_poly = None



                # Add tri-strip to element array
                if len(tri_strip):
                    best_collection['tri_strips'].append({'mesh':mesh, 'strip':tri_strip, 'strip_bones':tri_strip_bones})
                    good = True
                if tri_strip_bones_overflow:
                    tri_strip_bones = []
                    tri_strip_bones_overflow = False
            
        print("Mesh contains", len(mesh.polygons), "triangles")
        print("Vert count: (%d -> %d)\n" % (len(mesh.loops), opt_gpu_vert_count))
        
    

    # Generate binary vertex buffer of collection index
    def generate_vertex_buffer(self, index, endian_char):
        collection = self.collections[index]
        if not collection:
            return None
        
        # Positions output
        pos_out = []
        
        # Generate vert buffer struct
        vstruct = struct.Struct(endian_char + 'f')
        
        # If rigging, determine maximum number of bones in this collection
        max_bones = 0
        for i in range(len(collection['vertices'])):
            weight_count = 0
            if collection['vert_weights'][i]:
                weight_count = len(collection['vert_weights'][i])
            if weight_count > max_bones:
                max_bones = weight_count
        max_bones = ROUND_UP_4(max_bones)
        
        # Build byte array
        vert_bytes = bytearray()
        for i in range(len(collection['vertices'])):
            loop_vert = collection['vertices'][i]
            bloop = loop_vert[0]
            mesh = bloop.mesh
            bvert = mesh.vertices[bloop.loop.vertex_index]
            #print(bvert.co)

            # Position
            pos_out.append((bvert.co, bvert.normal))
            for comp in range(4):
                if comp in range(len(bvert.co)):
                    vert_bytes += vstruct.pack(bvert.co[comp])
                else:
                    vert_bytes += vstruct.pack(0.0)
                
            # Normal
            for comp in range(4):
                if comp in range(len(bvert.normal)):
                    vert_bytes += vstruct.pack(bvert.normal[comp])
                else:
                    vert_bytes += vstruct.pack(0.0)
                    
            # Weights
            weights = collection['vert_weights'][i]
            for j in range(max_bones):
                if j < len(weights):
                    vert_bytes += vstruct.pack(weights[j])
                else:
                    vert_bytes += vstruct.pack(0.0)
            
            # UVs
            added_uvs = 0
            for uv_idx in range(collection['uv_count']):
                coords = mesh.uv_layers[uv_idx].data[bloop.loop.index].uv
                vert_bytes += vstruct.pack(coords[0])
                vert_bytes += vstruct.pack(-coords[1])
                added_uvs += 1
            
            # Pad to 16-byte alignment
            if added_uvs & 1:
                vert_bytes += vstruct.pack(0.0)
                vert_bytes += vstruct.pack(0.0)


        return collection['uv_count'], max_bones, vert_bytes, pos_out
        
        
    # Generate binary element buffer of collection index
    def generate_element_buffer(self, index, endian_char):
        collection = self.collections[index]
        if not collection:
            return None
        
        # Numeric array out
        arr_out = []
        
        # Generate element buffer struct
        estruct = struct.Struct(endian_char + 'H')
        
        # Build mesh-primitive hierarchy
        last_mesh = collection['tri_strips'][0]['mesh']
        mesh_primitives = {'mesh':last_mesh, 'primitives':[]}
        collection_primitives = [mesh_primitives]
        
        # Collection element byte-array
        cur_offset = 0
        element_bytes = bytearray()
        
        # Last element index entry and strip length for forming degenerate strip
        last_elem = None
        strip_len = 0
        
        # Last strip bone array (for rigging)
        last_strip_bones = collection['tri_strips'][0]['strip_bones']

        # Build single degenerate tri-strip
        for strip in collection['tri_strips']:
            #print('new strip', collection['tri_strips'].index(strip))
            
            if last_mesh != strip['mesh'] or last_strip_bones != strip['strip_bones']:
                #print('splitting primitive')
                # New mesh; force new strip
                mesh_primitives['primitives'].append({'offset':cur_offset, 'length':strip_len, 'bones':last_strip_bones})
                cur_offset += strip_len
                last_elem = None
                strip_len = 0
                last_mesh = strip['mesh']
                mesh_primitives = {'mesh':last_mesh, 'primitives':[]}
                collection_primitives.append(mesh_primitives)
            
            elif last_elem is not None:
                #print('extending primitive')
                # Existing mesh being extended as degenerate strip
                strip_len += 2
                element_bytes += estruct.pack(last_elem)
                element_bytes += estruct.pack(strip['strip'][0])
                arr_out.append(last_elem)
                arr_out.append(strip['strip'][0])
            
                # If current element count is odd, add additional degenerate strip to make it even
                # This ensures that the sub-strip has proper winding-order for backface culling
                if (strip_len & 1):
                    strip_len += 1
                    element_bytes += estruct.pack(strip['strip'][0])
                    arr_out.append(strip['strip'][0])


            # Primitive tri-strip byte array
            for idx in strip['strip']:
                #print(idx)
                strip_len += 1
                element_bytes += estruct.pack(idx)
                arr_out.append(idx)
                last_elem = idx
    
        # Final mesh entry
        mesh_primitives['primitives'].append({'offset':cur_offset, 'length':strip_len, 'bones':last_strip_bones})
        cur_offset += strip_len
            
        return collection_primitives, element_bytes, arr_out


    # Generate binary draw-index buffer of collection index
    def generate_index_buffer(self, collection_primitives, endian_char, rigger):

        # Bytearray to fill
        index_bytes = bytearray()

        # Submesh count
        index_bytes += struct.pack(endian_char + 'I', len(collection_primitives))

        # And array
        for mesh in collection_primitives:
            
            # Primitive count
            index_bytes += struct.pack(endian_char + 'I', len(mesh['primitives']))

            # Primitive array
            for prim in mesh['primitives']:
                
                # If rigging, append skin index
                if rigger:
                    skin_index = rigger.augment_skin(prim['bones'])
                    index_bytes += struct.pack(endian_char + 'I', skin_index)
                
                index_bytes += struct.pack(endian_char + 'I', 2)
                index_bytes += struct.pack(endian_char + 'I', prim['offset'])
                index_bytes += struct.pack(endian_char + 'I', prim['length'])

        return index_bytes


# C-generation operator
import bmesh
class hmdl_mesh_operator(bpy.types.Operator):
    bl_idname = "scene.hmdl_mesh"
    bl_label = "HMDL C mesh maker"
    bl_description = "HMDL Mesh source generation utility"

    @classmethod
    def poll(cls, context):
        return context.object and context.object.type == 'MESH'

    def execute(self, context):
        copy_mesh = context.object.data.copy()
        copy_obj = context.object.copy()
        copy_obj.data = copy_mesh
    
        bm = bmesh.new()
        bm.from_mesh(copy_mesh)
        bmesh.ops.triangulate(bm, faces=bm.faces)
        #to_remove = []
        #for face in bm.faces:
        #    if face.material_index != 7:
        #        to_remove.append(face)
        #bmesh.ops.delete(bm, geom=to_remove, context=5)
        bm.to_mesh(copy_mesh)
        bm.free()
        
        context.scene.objects.link(copy_obj)
        rmesh = hmdl_mesh()
        rmesh.add_mesh(copy_mesh, None, 0)
        
        str_out = '/* Vertex Buffer */\nstatic const float VERT_BUF[] = {\n'
        vert_arr = rmesh.generate_vertex_buffer(0, '<')[3]
        for v in vert_arr:
            str_out += '    %f, %f, %f, 0.0, %f, %f, %f, 0.0,\n' % (v[0][0], v[0][1], v[0][2], v[1][0], v[1][1], v[1][2])
        ebuf_arr = rmesh.generate_element_buffer(0, '<')[2]
        str_out += '};\n\n/* Element Buffer */\n#define ELEM_BUF_COUNT %d\nstatic const u16 ELEM_BUF[] = {\n' % len(ebuf_arr)
        for e in ebuf_arr:
            str_out += '    %d,\n' % e
        str_out += '};\n'
        
        context.scene.objects.unlink(copy_obj)
        bpy.data.objects.remove(copy_obj)
        bpy.data.meshes.remove(copy_mesh)
        
        context.window_manager.clipboard = str_out
        self.report({'INFO'}, "Wrote mesh C to clipboard")
        return {'FINISHED'}
