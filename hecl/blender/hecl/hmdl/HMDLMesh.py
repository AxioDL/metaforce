'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>
'''

import bpy, bmesh, operator, struct
from mathutils import Vector

# Class for building unique sets of vertex attributes for VBO generation
class VertPool:

    # Initialize hash-unique index for each available attribute
    def __init__(self, bm, rna_loops):
        self.rna_loops = rna_loops
        self.pos = {}
        self.norm = {}
        self.skin = {}
        self.color = {}
        self.uv = {}
        self.dlay = None
        self.clays = []
        self.ulays = []

        dlay = None
        if len(bm.verts.layers.deform):
            dlay = bm.verts.layers.deform[0]
            self.dlay = dlay

        clays = []
        for cl in range(len(bm.loops.layers.color)):
            clays.append(bm.loops.layers.color[cl])
        self.clays = clays

        ulays = []
        for ul in range(len(bm.loops.layers.uv)):
            ulays.append(bm.loops.layers.uv[ul])
        self.ulays = ulays

        # Per-vert pool attributes
        for v in bm.verts:
            pf = v.co.copy().freeze()
            if pf not in self.pos:
                self.pos[pf] = len(self.pos)
            if not rna_loops:
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
                if rna_loops:
                    nf = rna_loops[l.index].normal.copy().freeze()
                    if nf not in self.norm:
                        self.norm[nf] = len(self.norm)
                for cl in range(len(clays)):
                    cf = l[clays[cl]].copy().freeze()
                    if cf not in self.color:
                        self.color[cf] = len(self.color)
                for ul in range(len(ulays)):
                    uf = l[ulays[ul]].uv.copy().freeze()
                    if uf not in self.uv:
                        self.uv[uf] = len(self.uv)

    def write_out(self, writebuf, vert_groups):
        writebuf(struct.pack('I', len(self.pos)))
        for p in sorted(self.pos.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', p[0][0], p[0][1], p[0][2]))

        writebuf(struct.pack('I', len(self.norm)))
        for n in sorted(self.norm.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', n[0][0], n[0][1], n[0][2]))

        writebuf(struct.pack('II', len(self.clays), len(self.color)))
        for c in sorted(self.color.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', c[0][0], c[0][1], c[0][2]))

        writebuf(struct.pack('II', len(self.ulays), len(self.uv)))
        for u in sorted(self.uv.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('ff', u[0][0], u[0][1]))

        writebuf(struct.pack('I', len(vert_groups)))
        for vgrp in vert_groups:
            writebuf((vgrp.name + '\n').encode())

        writebuf(struct.pack('I', len(self.skin)))
        for s in sorted(self.skin.items(), key=operator.itemgetter(1)):
            entries = s[0]
            writebuf(struct.pack('I', len(entries)))
            if len(entries):
                total_len = 0.0
                for ent in entries:
                    total_len += ent[1]
                for ent in entries:
                    writebuf(struct.pack('If', ent[0], ent[1] / total_len))

    def get_pos_idx(self, vert):
        pf = vert.co.copy().freeze()
        return self.pos[pf]

    def get_norm_idx(self, loop):
        if self.rna_loops:
            nf = self.rna_loops[loop.index].normal.copy().freeze()
        else:
            nf = loop.vert.normal.copy().freeze()
        return self.norm[nf]

    def get_skin_idx(self, vert):
        if not self.dlay:
            return 0
        sf = tuple(sorted(vert[self.dlay].items()))
        return self.skin[sf]

    def get_color_idx(self, loop, cidx):
        cf = loop[self.clays[cidx]].copy().freeze()
        return self.color[cf]

    def get_uv_idx(self, loop, uidx):
        uf = loop[self.ulays[uidx]].uv.copy().freeze()
        return self.uv[uf]

    def loop_out(self, writebuf, loop):
        writebuf(struct.pack('B', 1))
        writebuf(struct.pack('II', self.get_pos_idx(loop.vert), self.get_norm_idx(loop)))
        for cl in range(len(self.clays)):
            writebuf(struct.pack('I', self.get_color_idx(loop, cl)))
        for ul in range(len(self.ulays)):
            writebuf(struct.pack('I', self.get_uv_idx(loop, ul)))
        sp = struct.pack('I', self.get_skin_idx(loop.vert))
        writebuf(sp)

def sort_faces_by_skin_group(dlay, faces):
    faces_out = []
    done_sg = set()
    ref_sg = None
    while len(faces_out) < len(faces):
        for f in faces:
            found = False
            for v in f.verts:
                sg = tuple(sorted(v[dlay].items()))
                if sg not in done_sg:
                    ref_sg = sg
                    done_sg.add(ref_sg)
                    found = True
                    break
            if found:
                break

        for f in faces:
            if f in faces_out:
                continue
            for v in f.verts:
                sg = tuple(sorted(v[dlay].items()))
                if sg == ref_sg:
                    faces_out.append(f)
                    break

    return faces_out

def recursive_faces_islands(dlay, list_out, rem_list, skin_slot_set, skin_slot_count, face):
    if face not in rem_list:
        return []

    if dlay:
        for v in face.verts:
            sg = tuple(sorted(v[dlay].items()))
            if sg not in skin_slot_set:
                if skin_slot_count > 0 and len(skin_slot_set) == skin_slot_count:
                    return False
                skin_slot_set.add(sg)

    list_out.append(face)
    rem_list.remove(face)
    next_faces = []
    for e in face.edges:
        if not e.is_contiguous:
            continue
        for f in e.link_faces:
            if f == face:
                continue
            next_faces.append(f)
    return next_faces

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
        return None, None, None
    list_out.append(face)
    rem_list.remove(face)
    edge = find_opposite_edge(face, boot_edge, boot_edge_2, last_edge, last_edge_2)
    if not edge:
        return None, None, None
    for f in edge.link_faces:
        if f == face:
            continue
        return f, edge, last_edge
    return None, None, None

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

def stripify_primitive(writebuf, vert_pool, prim_faces, last_loop, next_idx):
    if last_loop:
        vert_pool.loop_out(writebuf, last_loop)
        next_idx += 1

    if len(prim_faces) == 1:
        loop = prim_faces[0].loops[0]
        if last_loop:
            vert_pool.loop_out(writebuf, loop)
            next_idx += 1
        if next_idx & 1:
            rev = True
        else:
            rev = False
        for i in range(3):
            vert_pool.loop_out(writebuf, loop)
            last_loop = loop
            next_idx += 1
            if rev:
                loop = loop.link_loop_prev
            else:
                loop = loop.link_loop_next
        return last_loop, next_idx

    loop = find_loop_opposite_from_other_face(prim_faces[0], prim_faces[1])
    if last_loop:
        vert_pool.loop_out(writebuf, loop)
        next_idx += 1
    if next_idx & 1:
        rev = True
    else:
        rev = False
    for i in range(3):
        vert_pool.loop_out(writebuf, loop)
        last_loop = loop
        next_idx += 1
        if rev:
            loop = loop.link_loop_prev
        else:
            loop = loop.link_loop_next

    for i in range(1, len(prim_faces)):
        loop = find_loop_opposite_from_other_face(prim_faces[i], prim_faces[i-1])
        vert_pool.loop_out(writebuf, loop)
        last_loop = loop
        next_idx += 1

    return last_loop, next_idx


def write_out_surface(writebuf, vert_pool, island_faces, mat_idx):

    # Centroid of surface
    centroid = Vector()
    for f in island_faces:
        centroid += f.calc_center_bounds()
    centroid /= len(island_faces)
    writebuf(struct.pack('fff', centroid[0], centroid[1], centroid[2]))

    # Material index
    writebuf(struct.pack('I', mat_idx))

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
    writebuf(struct.pack('fff', aabb_min[0], aabb_min[1], aabb_min[2]))
    writebuf(struct.pack('fff', aabb_max[0], aabb_max[1], aabb_max[2]))

    # Average normal of surface
    avg_norm = Vector()
    for f in island_faces:
        avg_norm += f.normal
    avg_norm.normalize()
    writebuf(struct.pack('fff', avg_norm[0], avg_norm[1], avg_norm[2]))

    # Count estimate
    writebuf(struct.pack('I', len(island_faces) * 3))

    # Verts themselves
    last_loop = None
    next_idx = 0
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
                    next_face = start_face
                    last_edge = None
                    last_edge_2 = None
                    while next_face is not None:
                        next_face, last_edge, last_edge_2 = recursive_faces_strip(sel_list, island_local, next_face,
                                                                                  e, e2, last_edge, last_edge_2)
                    if len(sel_list):
                        sel_lists_local.append(sel_list)
        max_count = 0
        max_sl = None
        for sl in sel_lists_local:
            if len(sl) > max_count:
                max_count = len(sl)
                max_sl = sl
        for f in max_sl:
            island_faces.remove(f)
        last_loop, next_idx = stripify_primitive(writebuf, vert_pool, max_sl, last_loop, next_idx)

    writebuf(struct.pack('B', 0))

