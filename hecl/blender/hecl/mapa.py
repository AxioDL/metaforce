import bpy, struct, bmesh
from . import hmdl
from mathutils import Vector
VertPool = hmdl.HMDLMesh.VertPool
strip_next_loop = hmdl.HMDLMesh.strip_next_loop

def cook(writebuf, mesh_obj):
    if mesh_obj.type != 'MESH':
        raise RuntimeError("%s is not a mesh" % mesh_obj.name)

    obj_vismodes = dict((i[0], i[3]) for i in bpy.types.Object.retro_mapobj_vis_mode[1]['items'])

    # Write out visibility type
    vis_types = dict((i[0], i[3]) for i in bpy.types.Scene.retro_map_vis_mode[1]['items'])
    writebuf(struct.pack('I', vis_types[bpy.context.scene.retro_map_vis_mode]))

    # Copy mesh (and apply mesh modifiers with triangulation)
    copy_name = mesh_obj.name + "_hmdltri"
    copy_mesh = bpy.data.meshes.new(copy_name)
    copy_obj = bpy.data.objects.new(copy_name, copy_mesh)
    copy_obj.data = mesh_obj.to_mesh(bpy.context.scene, True, 'RENDER')
    copy_mesh = copy_obj.data
    copy_obj.scale = mesh_obj.scale
    bpy.context.scene.objects.link(copy_obj)
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.scene.objects.active = copy_obj
    copy_obj.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.quads_convert_to_tris()
    bpy.ops.mesh.select_all(action='DESELECT')
    bpy.context.scene.update()
    bpy.ops.object.mode_set(mode='OBJECT')
    copy_mesh.calc_normals_split()
    rna_loops = copy_mesh.loops

    # Create master BMesh and VertPool
    bm_master = bmesh.new()
    bm_master.from_mesh(copy_obj.data)
    vert_pool = VertPool(bm_master, rna_loops)

    # Output vert pool
    vert_pool.write_out_map(writebuf)

    # Create map surfaces and borders
    island_faces = list(bm_master.faces)
    #prev_loop_emit = None
    loop_ranges = []
    loop_iter = 0
    while len(island_faces):
        out_count = 0
        sel_lists_local = []
        restore_out_count = out_count
        for start_face in island_faces:
            for l in start_face.loops:
                out_count = restore_out_count
                island_local = list(island_faces)
                if out_count & 1:
                    prev_loop = l.link_loop_prev
                    loop = prev_loop.link_loop_prev
                    sel_list = [l, prev_loop, loop]
                    prev_loop = loop
                else:
                    prev_loop = l.link_loop_next
                    loop = prev_loop.link_loop_next
                    sel_list = [l, prev_loop, loop]
                out_count += 3
                island_local.remove(start_face)
                while True:
                    if not prev_loop.edge.is_contiguous or prev_loop.edge.seam:
                        break
                    loop, prev_loop = strip_next_loop(prev_loop, out_count)
                    face = loop.face
                    if face not in island_local:
                        break
                    sel_list.append(loop)
                    island_local.remove(face)
                    out_count += 1
                sel_lists_local.append((sel_list, island_local, out_count))
        max_count = 0
        max_sl = None
        max_island_faces = None
        for sl in sel_lists_local:
            if len(sl[0]) > max_count:
                max_count = len(sl[0])
                max_sl = sl[0]
                max_island_faces = sl[1]
                out_count = sl[2]
        island_faces = max_island_faces

        loop_set = set()
        edge_set = set()
        loop_count = len(max_sl)
        for loop in max_sl:
            vert_pool.loop_out_map(writebuf, loop)
            loop_set.add(loop)
            for edge in loop.face.edges:
                if edge.seam:
                    edge_set.add(edge)

        if len(edge_set):
            trace_edge = edge_set.pop()
        else:
            trace_edge = None
        edge_ranges = []
        edge_iter = loop_iter + loop_count
        while trace_edge:
            edge_count = 0
            vert_pool.vert_out_map(writebuf, trace_edge.verts[0])
            vert_pool.vert_out_map(writebuf, trace_edge.verts[1])
            edge_count += 2
            next_vert = trace_edge.verts[1]
            found_edge = True
            while found_edge:
                found_edge = False
                for edge in next_vert.link_edges:
                    if edge in edge_set:
                        edge_set.remove(edge)
                        next_vert = edge.other_vert(next_vert)
                        vert_pool.vert_out_map(writebuf, next_vert)
                        edge_count += 1
                        found_edge = True
                        break
            if len(edge_set):
                trace_edge = edge_set.pop()
            else:
                trace_edge = None
            edge_ranges.append((edge_iter, edge_count))
            edge_iter += edge_count

        pos_avg = Vector()
        norm_avg = Vector()
        if len(loop_set):
            for loop in loop_set:
                pos_avg += loop.vert.co
                norm_avg += loop.vert.normal
            pos_avg /= len(loop_set)
            norm_avg /= len(loop_set)
            norm_avg.normalize()

        loop_ranges.append((loop_iter, loop_count, edge_ranges, pos_avg, norm_avg))
        loop_iter = edge_iter

    # No more surfaces
    writebuf(struct.pack('B', 0))

    # Write out loop ranges and averages
    writebuf(struct.pack('I', len(loop_ranges)))
    for loop_range in loop_ranges:
        writebuf(struct.pack('fff', loop_range[3][0], loop_range[3][1], loop_range[3][2]))
        writebuf(struct.pack('fff', loop_range[4][0], loop_range[4][1], loop_range[4][2]))
        writebuf(struct.pack('II', loop_range[0], loop_range[1]))
        writebuf(struct.pack('I', len(loop_range[2])))
        for edge_range in loop_range[2]:
            writebuf(struct.pack('II', edge_range[0], edge_range[1]))

    # Write out mappable objects
    poi_count = 0
    for obj in bpy.context.scene.objects:
        if obj.retro_mappable_type != -1:
            poi_count += 1

    writebuf(struct.pack('I', poi_count))
    for obj in bpy.context.scene.objects:
        if obj.retro_mappable_type != -1:
            writebuf(struct.pack('III',
                                 obj.retro_mappable_type, obj_vismodes[obj.retro_mapobj_vis_mode], int(obj.retro_mappable_sclyid, 0)))
            writebuf(struct.pack('ffffffffffffffff',
                                 obj.matrix_world[0][0], obj.matrix_world[0][1], obj.matrix_world[0][2], obj.matrix_world[0][3],
                                 obj.matrix_world[1][0], obj.matrix_world[1][1], obj.matrix_world[1][2], obj.matrix_world[1][3],
                                 obj.matrix_world[2][0], obj.matrix_world[2][1], obj.matrix_world[2][2], obj.matrix_world[2][3],
                                 obj.matrix_world[3][0], obj.matrix_world[3][1], obj.matrix_world[3][2], obj.matrix_world[3][3]))

def draw(layout, context):
    obj = context.active_object
    layout.prop(context.scene, 'retro_map_vis_mode', text='Visibility Mode')
    if obj and obj.retro_mappable_type != -1:
        layout.prop(obj, 'retro_mappable_type', text='Object Type')
        layout.prop(obj, 'retro_mapobj_vis_mode', text='Object Visibility Mode')
        layout.prop(obj, 'retro_mappable_sclyid', text='Object ID')

def register():
    bpy.types.Object.retro_mappable_type = bpy.props.IntProperty(name='Retro: MAPA object type', default=-1)
    bpy.types.Object.retro_mappable_sclyid = bpy.props.StringProperty(name='Retro: MAPA object SCLY ID')
    bpy.types.Scene.retro_map_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', 0),
    ('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),
    ('VISIT', 'Visit', 'Visible after Visit', 2),
    ('NEVER', 'Never', 'Never Visible', 3)],
    name='Retro: Map Visibility Mode')
    bpy.types.Object.retro_mapobj_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', 0),
    ('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),
    ('VISIT', 'Visit', 'Visible after Door Visit', 2),
    ('NEVER', 'Never', 'Never Visible', 3),
    ('MAPSTATIONORVISIT2', 'Map Station or Visit 2', 'Visible after Map Station or Visit', 4)],
    name='Retro: Map Object Visibility Mode')
