import bpy, bmesh, operator, struct
from mathutils import Vector

# Class for building unique sets of vertex attributes for VBO generation
class VertPool:

    # Initialize hash-unique index for each available attribute
    def __init__(self, bm, rna_loops):
        self.bm = bm
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

    def loops_contiguous(self, la, lb):
        if la.vert != lb.vert:
            return False
        if self.get_norm_idx(la) != self.get_norm_idx(lb):
            return False
        for cl in range(len(self.clays)):
            if self.get_color_idx(la, cl) != self.get_color_idx(lb, cl):
                return False
        for ul in range(len(self.ulays)):
            if self.get_uv_idx(la, ul) != self.get_uv_idx(lb, ul):
                return False
        return True

    def splitable_edge(self, edge):
        if len(edge.link_faces) < 2:
            return False
        for v in edge.verts:
            found = None
            for f in edge.link_faces:
                for l in f.loops:
                    if l.vert == v:
                        if not found:
                            found = l
                            break
                        else:
                            if not self.loops_contiguous(found, l):
                                return True
                            break
        return False

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

def strip_next_loop(prev_loop, out_count):
    if out_count & 1:
        radial_loop = prev_loop.link_loop_radial_next
        loop = radial_loop.link_loop_prev
        return loop, loop
    else:
        radial_loop = prev_loop.link_loop_radial_prev
        loop = radial_loop.link_loop_next
        return loop.link_loop_next, loop

def write_out_surface(writebuf, output_mode, vert_pool, island_faces, mat_idx):

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

    # Count estimate (as raw triangles)
    writebuf(struct.pack('I', len(island_faces) * 3))

    # Verts themselves
    if output_mode == 'TRIANGLES':
        for f in island_faces:
            for l in f.loops:
                vert_pool.loop_out(writebuf, l)

    elif output_mode == 'TRISTRIPS':
        prev_loop_emit = None
        out_count = 0
        while len(island_faces):
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
                        if not prev_loop.edge.is_contiguous or prev_loop.edge.tag:
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
            if prev_loop_emit:
                vert_pool.loop_out(writebuf, prev_loop_emit)
                vert_pool.loop_out(writebuf, max_sl[0])
            for loop in max_sl:
                vert_pool.loop_out(writebuf, loop)
                prev_loop_emit = loop

    writebuf(struct.pack('B', 0))

