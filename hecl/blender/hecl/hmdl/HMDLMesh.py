'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>
'''

import bpy, bmesh, operator, struct
from mathutils import Vector

# Class for building unique sets of vertex attributes for VBO generation
class VertPool:
    pos = {}
    norm = {}
    skin = {}
    color = []
    uv = []
    dlay = None
    clays = []
    ulays = []

    # Initialize hash-unique index for each available attribute
    def __init__(self, bm):
        dlay = None
        if len(bm.verts.layers.deform):
            dlay = bm.verts.layers.deform[0]
            self.dlay = dlay

        clays = []
        for cl in range(len(bm.loops.layers.color)):
            clays.append(bm.loops.layers.color[cl])
            self.color.append([])
        self.clays = clays

        ulays = []
        for ul in range(len(bm.loops.layers.uv)):
            ulays.append(bm.loops.layers.uv[ul])
            self.uv.append([])
        self.ulays = ulays

        # Per-vert pool attributes
        for v in bm.verts:
            pf = v.co.copy().freeze()
            if pf not in self.pos:
                self.pos[pf] = len(self.pos)
            nf = v.normal.copy().freeze()
            if nf not in self.norm:
                self.norm[nf] = len(self.norm)
            if dlay:
                sf = tuple(sorted(v[dlay].items()))
                if sf not in self.skin:
                    self.skin[sf] = len(self.skin)

        # Per-loop pool attributes
        for f in bm.faces:
            for l in f.loops:
                for cl in range(len(clays)):
                    cf = l[clays[cl]].copy().freeze()
                    if cf not in self.color[cl]:
                        self.color[cl][cf] = len(self.color[cl])
                for ul in range(len(ulays)):
                    uf = l[ulays[ul]].uv.copy().freeze()
                    if uf not in self.uv[ul]:
                        self.uv[ul][uf] = len(self.uv[ul])

    def write_out(self, writebuffunc, vert_groups):
        writebuffunc(struct.pack('I', len(self.pos)))
        for p in sorted(self.pos.items(), key=operator.itemgetter(1)):
            writebuffunc(struct.pack('fff', p[0][0], p[0][1], p[0][2]))

        writebuffunc(struct.pack('I', len(self.norm)))
        for n in sorted(self.norm.items(), key=operator.itemgetter(1)):
            writebuffunc(struct.pack('fff', n[0][0], n[0][1], n[0][2]))

        writebuffunc(struct.pack('I', len(self.color)))
        for clay in self.color:
            writebuffunc(struct.pack('I', len(clay)))
            for c in sorted(clay.items(), key=operator.itemgetter(1)):
                writebuffunc(struct.pack('fff', c[0][0], c[0][1], c[0][2]))

        writebuffunc(struct.pack('I', len(self.uv)))
        for ulay in self.uv:
            writebuffunc(struct.pack('I', len(ulay)))
            for u in sorted(ulay.items(), key=operator.itemgetter(1)):
                writebuffunc(struct.pack('ff', u[0][0], u[0][1]))

        writebuffunc(struct.pack('I', len(vert_groups)))
        for vgrp in vert_groups:
            writebuffunc((vgrp.name + '\n').encode())

        writebuffunc(struct.pack('I', len(self.skin)))
        for s in sorted(self.skin.items(), key=operator.itemgetter(1)):
            entries = s[0]
            writebuffunc(struct.pack('I', len(entries)))
            for ent in entries:
                writebuffunc(struct.pack('If', ent[0], ent[1]))

    def set_bm_layers(self, bm):
        self.dlay = None
        if len(bm.verts.layers.deform):
            self.dlay = bm.verts.layers.deform[0]

        clays = []
        for cl in range(len(bm.loops.layers.color)):
            clays.append(bm.loops.layers.color[cl])
        self.clays = clays

        ulays = []
        for ul in range(len(bm.loops.layers.uv)):
            ulays.append(bm.loops.layers.uv[ul])
        self.ulays = ulays

    def get_pos_idx(self, vert):
        pf = vert.co.copy().freeze()
        return self.pos[pf]

    def get_norm_idx(self, vert):
        nf = vert.normal.copy().freeze()
        return self.norm[nf]

    def get_skin_idx(self, vert):
        sf = tuple(sorted(vert[self.dlay].items()))
        return self.skin[sf]

    def get_color_idx(self, loop, cidx):
        cf = tuple(sorted(loop[self.clays[cidx]].items()))
        return self.color[cidx][cf]

    def get_uv_idx(self, loop, uidx):
        uf = tuple(sorted(loop[self.ulays[uidx]].items()))
        return self.uv[uidx][uf]

    def loop_out(self, writebuffunc, loop):
        writebuffunc(struct.pack('BII', 1, self.get_pos_idx(loop.vert), self.get_norm_idx(loop.vert)))
        for cl in range(len(self.clays)):
            writebuffunc('I', self.get_color_idx(loop, cl))
        for ul in range(len(self.ulays)):
            writebuffunc('I', self.get_uv_idx(loop, ul))
        writebuffunc(struct.pack('I', self.get_skin_idx(loop.vert)))

def recursive_faces_islands(dlay, list_out, rem_list, skin_slot_set, skin_slot_count, face):
    if face not in rem_list:
        return None

    if dlay:
        for v in face.verts:
            sg = tuple(sorted(v[dlay].items()))
            if sg not in skin_slot_set and len(skin_slot_set) == skin_slot_count:
                return False
            skin_slot_set.add(sg)

    list_out.append(face)
    rem_list.remove(face)
    for e in face.edges:
        if not e.is_contiguous:
            continue
        for f in e.link_faces:
            if f == face:
                continue
            if recursive_faces_islands(dlay, list_out, rem_list, skin_slot_set, skin_slot_count, f) == False:
                return False

def find_opposite_edge(face, boot_edge, boot_edge2, last_edge, last_edge_2):
    if last_edge_2:
        for e in face.edges:
            if e.verts[0] in last_edge_2.verts or e.verts[1] in last_edge_2.verts:
                continue
            return e
    elif last_edge:
        return boot_edge2
    else:
        return boot_edge

def recursive_faces_strip(list_out, rem_list, face, boot_edge, boot_edge_2, last_edge, last_edge_2):
    if face not in rem_list:
        return
    list_out.append(face)
    rem_list.remove(face)
    edge = find_opposite_edge(face, boot_edge, boot_edge_2, last_edge, last_edge_2)
    if not edge:
        return
    for f in edge.link_faces:
        if f == face:
            continue
        recursive_faces_strip(list_out, rem_list, f, boot_edge, boot_edge_2, edge, last_edge)
        break

def count_contiguous_edges(face):
    retval = 0
    for e in face.edges:
        if e.is_contiguous:
            retval += 1
    return retval

def find_loop_opposite_from_other_face(face, other_face):
    for e in face.edges:
        if e in other_face.edges:
            edge = e
            break
    for l in face.loops:
        if l.vert in edge.verts:
            continue
        return l

def stripify_primitive(writebuffunc, vert_pool, prim_faces, last_loop, last_idx):
    if last_loop:
        vert_pool.loop_out(writebuffunc, last_loop)
        last_idx += 1

    if len(prim_faces) == 1:
        loop = prim_faces[0].loops[0]
        if last_loop:
            vert_pool.loop_out(writebuffunc, loop)
            last_idx += 1
        if last_idx & 1:
            rev = True
        else:
            rev = False
        for i in range(3):
            vert_pool.loop_out(writebuffunc, loop)
            last_loop = loop
            last_idx += 1
            if rev:
                loop = loop.link_loop_prev
            else:
                loop = loop.link_loop_next
        return last_loop, last_idx

    loop = find_loop_opposite_from_other_face(prim_faces[0], prim_faces[1])
    if last_loop:
        vert_pool.loop_out(writebuffunc, loop)
        last_idx += 1
    if last_idx & 1:
        rev = True
    else:
        rev = False
    for i in range(3):
        vert_pool.loop_out(writebuffunc, loop)
        last_loop = loop
        last_idx += 1
        if rev:
            loop = loop.link_loop_prev
        else:
            loop = loop.link_loop_next

    for i in range(1, len(prim_faces)):
        loop = find_loop_opposite_from_other_face(prim_faces[i], prim_faces[i-1])
        vert_pool.loop_out(writebuffunc, loop)
        last_loop = loop
        last_idx += 1

    return last_loop, last_idx


def write_out_surface(writebuffunc, vert_pool, bm, island_faces, mat_idx):

    # Centroid of surface
    centroid = Vector()
    for f in island_faces:
        centroid += f.calc_center_bounds()
    centroid /= len(island_faces)
    writebuffunc(struct.pack('fff', centroid[0], centroid[1], centroid[2]))

    # Material index
    writebuffunc(struct.pack('I', mat_idx))

    # AABB of surface
    aabb_min = Vector((9999999, 9999999, 9999999))
    aabb_max = Vector((-9999999, -9999999, -9999999))
    for f in island_faces:
        for v in f.verts:
            for c in range(3):
                if v.co[c] < aabb_min[c]:
                    aabb_min[c] = v.co[c]
                if v.co[c] > aabb_max[c]:
                    aabb_max[c] = v.co[c]
    writebuffunc(struct.pack('fff', aabb_min[0], aabb_min[1], aabb_min[2]))
    writebuffunc(struct.pack('fff', aabb_max[0], aabb_max[1], aabb_max[2]))

    # Average normal of surface
    avg_norm = Vector()
    for f in island_faces:
        avg_norm += f.normal
    avg_norm.normalize()
    writebuffunc(struct.pack('fff', avg_norm[0], avg_norm[1], avg_norm[2]))

    # Verts themselves
    last_loop = None
    last_idx = 0
    while len(island_faces):
        sel_lists_local = []
        for start_face in island_faces:
            for e in start_face.edges:
                next_edges = []
                for f in e.link_faces:
                    if f == start_face:
                        continue
                    for eg in f.edges:
                        if eg == e:
                            continue
                        next_edges.append(eg)
                    break
                if len(next_edges) == 0:
                    next_edges.append(None)
                for e2 in next_edges:
                    island_local = list(island_faces)
                    sel_list = []
                    recursive_faces_strip(sel_list, island_local, start_face, e, e2, None, None)
                    sel_lists_local.append(sel_list)
        max_count = 0
        max_sl = None
        for sl in sel_lists_local:
            if len(sl) > max_count:
                max_count = len(sl)
                max_sl = sl
        for f in max_sl:
            island_faces.remove(f)
        last_loop, last_idx = stripify_primitive(writebuffunc, vert_pool, max_sl, last_loop, last_idx)
    writebuffunc(struct.pack('B', 0))

