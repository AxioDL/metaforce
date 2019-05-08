import bpy, struct
from mathutils import Vector

def build_dock_connections(scene):
    areas = []
    docks = []

    for obj in sorted(scene.objects, key=lambda x: x.name):
        if obj.type == 'MESH' and obj.parent is None:
            dock_list = []
            for ch in obj.children:
                if ch.type == 'MESH':
                    docks.append((len(areas), len(dock_list), ch))
                    dock_list.append(ch)
            areas.append((obj, dock_list))

    dock_dict = dict()

    for dockA in docks:
        mtxA = dockA[2].matrix_world
        locA = Vector((mtxA[0][3], mtxA[1][3], mtxA[2][3]))
        match = False
        for dockB in docks:
            if dockA == dockB:
                continue
            mtxB = dockB[2].matrix_world
            locB = Vector((mtxB[0][3], mtxB[1][3], mtxB[2][3]))
            if (locA - locB).magnitude < 0.1:
                dock_dict[dockA[2].name] = dockB
                match = True
                break
        #if not match:
        #    raise RuntimeError('No dock match for %s' % dockA[2].name)

    return (areas, dock_dict)

# Cook
def cook(writebuf):
    areas, dock_conns = build_dock_connections(bpy.context.scene)
    writebuf(struct.pack('I', len(areas)))
    for area in areas:
        obj = area[0]
        dock_list = area[1]
        writebuf(struct.pack('I', len(obj.name)))
        writebuf(obj.name.encode())

        pt = Vector(obj.bound_box[0])
        writebuf(struct.pack('fff', pt[0], pt[1], pt[2]))
        pt = Vector(obj.bound_box[6])
        writebuf(struct.pack('fff', pt[0], pt[1], pt[2]))

        wmtx = obj.matrix_world
        writebuf(struct.pack('ffffffffffffffff',
        wmtx[0][0], wmtx[0][1], wmtx[0][2], wmtx[0][3],
        wmtx[1][0], wmtx[1][1], wmtx[1][2], wmtx[1][3],
        wmtx[2][0], wmtx[2][1], wmtx[2][2], wmtx[2][3],
        wmtx[3][0], wmtx[3][1], wmtx[3][2], wmtx[3][3]))

        wmtx_inv = wmtx.inverted()

        writebuf(struct.pack('I', len(dock_list)))
        for ch in dock_list:
            if len(ch.data.vertices) < 4:
                raise RuntimeError('Not enough vertices in dock %s' % ch.name)
            wmtx = wmtx_inv @ ch.matrix_world
            for vi in range(4):
                v = wmtx @ ch.data.vertices[vi].co
                writebuf(struct.pack('fff', v[0], v[1], v[2]))
            if ch.name in dock_conns:
                conn_dock = dock_conns[ch.name]
                writebuf(struct.pack('I', conn_dock[0]))
                writebuf(struct.pack('I', conn_dock[1]))
            else:
                writebuf(struct.pack('I', 0xffffffff))
                writebuf(struct.pack('I', 0xffffffff))

# Panel draw
def draw(layout, context):
    pass
