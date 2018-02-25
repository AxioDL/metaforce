import bpy, bgl, sys, bmesh, struct
from mathutils import Vector

# Convenience class that automatically brings active edit mesh's face into scope for get/set
class HeightRef:
    def __init__(self):
        self.bm = None
        context = bpy.context
        obj = context.scene.objects[context.scene.hecl_path_obj]
        if obj.type != 'MESH':
            return
        if context.edit_object != obj:
            return
        bm = bmesh.from_edit_mesh(obj.data)
        if 'Height' not in bm.faces.layers.float:
            return
        self.height_lay = bm.faces.layers.float['Height']
        self.bm = bm

    @property
    def ready(self):
        return self.bm is not None and self.bm.faces.active is not None

    @property
    def value(self):
        if self.ready:
            return self.bm.faces.active[self.height_lay]

    @value.setter
    def value(self, value):
        if self.ready:
            for f in self.bm.faces:
                if f.select:
                    f[self.height_lay] = value

# Active edit face height get
def get_height(self):
    return HeightRef().value

# Selected edit face(s) height set
def set_height(self, val):
    HeightRef().value = val
    for ar in bpy.context.screen.areas:
        if ar.type == 'VIEW_3D':
            ar.tag_redraw()

# Edit panel
def draw(layout, context):
    layout.prop_search(context.scene, 'hecl_path_obj', context.scene, 'objects')
    layout.operator('view3d.toggle_path_height_visualization', text='Toggle Height Viz', icon='MANIPUL')
    layout.operator('view3d.toggle_path_background_wireframe', text='Toggle Background Wire', icon='WIRE')
    hr = HeightRef()
    if hr.ready:
        layout.prop(context.window_manager, 'hecl_height_prop', text='Height')

# Simple AABB class
class AABB:
    def __init__(self):
        self.min = Vector((999999.0, 999999.0, 999999.0))
        self.max = Vector((-999999.0, -999999.0, -999999.0))
    def accumulate(self, vec):
        for i in range(3):
            if vec[i] < self.min[i]:
                self.min[i] = vec[i]
            if vec[i] > self.max[i]:
                self.max[i] = vec[i]
    def isValid(self):
        for i in range(3):
            if self.min[i] > self.max[i]:
                return False
        return True

# Simple adjacency calculator
class AdjacencySet:
    def __init__(self, bm, mesh_obj, type_mask):
        self.faces = {}
        for f in bm.faces:
            material = mesh_obj.material_slots[f.material_index].material
            if (material.retro_path_type_mask & type_mask) == 0:
                continue
            face_set = set()
            face_set.add(f)

            # Breadth-first loop to avoid crashing python with large recursion
            next_level = [f]
            while len(next_level):
                next_next_level = []
                for of in next_level:
                    for e in of.edges:
                        for of2 in e.link_faces:
                            if of2 == of:
                                continue
                            if of2 not in face_set:
                                material = mesh_obj.material_slots[of2.material_index].material
                                if material.retro_path_type_mask & type_mask:
                                    face_set.add(of2)
                                    next_next_level.append(of2)
                next_level = next_next_level
            self.faces[f] = face_set

    def has_adjacency(self, face_a, face_b):
        if face_a not in self.faces:
            return False
        return face_b in self.faces[face_a]

# Cooking entry point
def cook(writebuf, mesh_obj):
    ba = bytearray()
    # Version 4
    ba += struct.pack('>I', 4)

    bm = bmesh.new()
    bm.from_mesh(mesh_obj.data)
    bm.faces.ensure_lookup_table()
    height_lay = None
    if 'Height' in bm.faces.layers.float:
        height_lay = bm.faces.layers.float['Height']

    # Gather immediate adjacencies
    node_list = []
    link_list = []
    region_list = []
    for f in bm.faces:
        start_loop = f.loops[0]
        cur_loop = start_loop
        node_idx = 0
        start_node = len(node_list)
        start_link = len(link_list)
        while cur_loop != start_loop:
            node_list.append(cur_loop)
            cur_loop = cur_loop.link_loop_prev
            for other_face in cur_loop.edge.link_faces:
                if other_face == f:
                    continue
                link_list.append((node_idx, other_face.index, cur_loop.edge.calc_length()))
            node_idx += 1
        region_list.append((f, range(start_node, len(node_list)), range(start_link, len(link_list))))

    # Emit nodes
    ba += struct.pack('>I', len(node_list))
    for n in node_list:
        v = n.vert
        normal = (n.edge.other_vert(v).co - v.co).normalized()
        ba += struct.pack('>ffffff', v.co[0], v.co[1], v.co[2], normal[0], normal[1], normal[2])

    # Emit links
    ba += struct.pack('>I', len(link_list))
    for l in link_list:
        ba += struct.pack('>IIff', l[0], l[1], l[2], 1.0 / l[2])

    # Emit regions
    ba += struct.pack('>I', len(region_list))
    for r in region_list:
        material = mesh_obj.material_slots[r[0].material_index].material
        height = 1.0
        if height_lay is not None:
            height = r[0][height_lay]
        center = r[0].calc_center_median_weighted()
        aabb = AABB()
        for v in r[0].verts:
            aabb.accumulate(v.co)
            aabb.accumulate(v.co + Vector(0.0, 0.0, height))
        ba += struct.pack('>IIIIHHffffIfffffffffI', len(r[1]), r[1].start, len(r[2]), r[2].start,
                          material.retro_path_idx_mask, material.retro_path_type_mask,
                          height, r[0].normal[0], r[0].normal[1], r[0].normal[2],
                          r[0].index, center[0], center[1], center[2],
                          aabb.min[0], aabb.min[1], aabb.min[2],
                          aabb.max[0], aabb.max[1], aabb.max[2],
                          r[0].index)

    num_regions = len(region_list)
    total_adjacencies = num_regions * (num_regions - 1) // 2
    num_words = (total_adjacencies + 31) / 32

    # Find ground adjacencies
    words = [0] * num_words
    ground_adjacencies = AdjacencySet(bm, mesh_obj, 0x1)
    for i in range(num_regions):
        for j in range(num_regions):
            if i == j:
                continue
            i1 = i
            i2 = j
            if i1 > i2:
                continue
            rem_regions = num_regions - i1
            rem_connections = rem_regions * (rem_regions - 1) // 2
            if ground_adjacencies.has_adjacency(bm.faces[i], bm.faces[j]):
                bit = total_adjacencies - rem_connections + i2 - (i1 + 1)
                words[bit / 32] |= 1 << (bit % 32)
    for w in words:
        ba += struct.pack('>I', w)

    # Find flyer adjacencies
    words = [0] * num_words
    flyer_adjacencies = AdjacencySet(bm, mesh_obj, 0x3)
    for i in range(num_regions):
        for j in range(num_regions):
            if i == j:
                continue
            i1 = i
            i2 = j
            if i1 > i2:
                continue
            rem_regions = num_regions - i1
            rem_connections = rem_regions * (rem_regions - 1) // 2
            if flyer_adjacencies.has_adjacency(bm.faces[i], bm.faces[j]):
                bit = total_adjacencies - rem_connections + i2 - (i1 + 1)
                words[bit / 32] |= 1 << (bit % 32)
    for w in words:
        ba += struct.pack('>I', w)

    # Unused zero bits
    for i in range((((num_regions * num_regions) + 31) / 32 - num_words) * 2):
        ba += struct.pack('>I', 0)

    # Empty octree (to be generated by hecl)
    ba += struct.pack('>II', 0, 0)

    # Write out
    writebuf(struct.pack('I', len(ba)))
    writebuf(ba)

# Line draw helper
def draw_line_3d(color, start, end):
    bgl.glColor4f(*color)
    bgl.glBegin(bgl.GL_LINES)
    bgl.glVertex3f(*start)
    bgl.glVertex3f(*end)

# Viewport hook callback
def draw_callback_3d(self, context):
    # object locations
    if context.scene.hecl_path_obj not in context.scene.objects:
        return
    obj = context.scene.objects[context.scene.hecl_path_obj]
    if obj.type != 'MESH':
        return
    obj_mtx = obj.matrix_world

    height_lay = None
    height_lambda = None
    if obj == context.edit_object:
        bm = bmesh.from_edit_mesh(obj.data)
        if 'Height' in bm.faces.layers.float:
            height_lay = bm.faces.layers.float['Height']
        height_lambda = lambda i: bm.faces[i][height_lay]
    else:
        if 'Height' in obj.data.polygon_layers_float:
            height_lay = obj.data.polygon_layers_float['Height']
        height_lambda = lambda i: height_lay.data[i].value

    for p in obj.data.polygons:
        height = 1.0
        if height_lay is not None:
            height = height_lambda(p.index)
        for ek in p.edge_keys:
            co0 = obj_mtx * obj.data.vertices[ek[0]].co
            co1 = obj_mtx * obj.data.vertices[ek[1]].co
            draw_line_3d((0.0, 0.0, 1.0, 0.7), co0 + Vector((0.0, 0.0, height)),
                         co1 + Vector((0.0, 0.0, height)))
        for vk in p.vertices:
            co = obj_mtx * obj.data.vertices[vk].co
            draw_line_3d((1.0, 0.0, 0.0, 0.7), co, co + Vector((0.0, 0.0, height)))

    bgl.glEnd()
    # restore opengl defaults
    bgl.glColor4f(0.0, 0.0, 0.0, 1.0)

# Toggle height viz button
class PathHeightDrawOperator(bpy.types.Operator):
    bl_idname = "view3d.toggle_path_height_visualization"
    bl_label = "Toggle PATH height visualization"
    _handle_3d = None

    def execute(self, context):
        # the arguments we pass the the callback
        args = (self, context)
        # Add the region OpenGL drawing callback
        # draw in view space with 'POST_VIEW' and 'PRE_VIEW'
        if self._handle_3d is None:
            PathHeightDrawOperator._handle_3d = \
                bpy.types.SpaceView3D.draw_handler_add(draw_callback_3d, args, 'WINDOW', 'POST_VIEW')
        else:
            bpy.types.SpaceView3D.draw_handler_remove(PathHeightDrawOperator._handle_3d, 'WINDOW')
            PathHeightDrawOperator._handle_3d = None

        for ar in bpy.context.screen.areas:
            if ar.type == 'VIEW_3D':
                ar.tag_redraw()
        return {'FINISHED'}

# Toggle background wire button
class PathBackgroundWireframeOperator(bpy.types.Operator):
    bl_idname = "view3d.toggle_path_background_wireframe"
    bl_label = "Toggle PATH background wireframe"
    _handle_3d = None

    def execute(self, context):
        if context.scene.background_set:
            to_wire = False
            for o in context.scene.background_set.objects:
                if o.draw_type != 'WIRE':
                    to_wire = True
                    break
            if to_wire:
                for o in context.scene.background_set.objects:
                    o.draw_type = 'WIRE'
            else:
                for o in context.scene.background_set.objects:
                    o.draw_type = 'TEXTURED'
        return {'FINISHED'}

# Registration
def register():
    bpy.types.Material.retro_path_idx_mask = bpy.props.IntProperty(name='Retro: Path Index Mask')
    bpy.types.Material.retro_path_type_mask = bpy.props.IntProperty(name='Retro: Path Type Mask')
    bpy.types.WindowManager.hecl_height_prop = bpy.props.FloatProperty(
        get=get_height, set=set_height, options={'HIDDEN'})
    bpy.types.Scene.hecl_path_obj = bpy.props.StringProperty(
        name='HECL Path Object',
        description='Blender Mesh Object to export during PATH\'s cook process')
    bpy.utils.register_class(PathHeightDrawOperator)
    bpy.utils.register_class(PathBackgroundWireframeOperator)

def unregister():
    bpy.utils.unregister_class(PathHeightDrawOperator)
    bpy.utils.unregister_class(PathBackgroundWireframeOperator)
