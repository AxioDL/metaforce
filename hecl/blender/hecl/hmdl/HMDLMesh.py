import bpy, bmesh, operator, struct

# Function to quantize normals to 15-bit precision
def quant_norm(n):
    nf = n.copy()
    for i in range(3):
        nf[i] = int(nf[i] * 16384) / 16384.0
    return nf.freeze()

# Function to quantize lightmap UVs to 15-bit precision
def quant_luv(n):
    uf = n.copy()
    for i in range(2):
        uf[i] = int(uf[i] * 32768) / 32768.0
    return uf.freeze()

# Function to output all mesh attribute values
def write_mesh_attrs(writebuf, bm, rna_loops, use_luv, material_slots):
    dlay = None
    if len(bm.verts.layers.deform):
        dlay = bm.verts.layers.deform[0]

    clays = []
    for cl in range(len(bm.loops.layers.color)):
        clays.append(bm.loops.layers.color[cl])
    writebuf(struct.pack('I', len(clays)))

    luvlay = None
    if use_luv:
        luvlay = bm.loops.layers.uv[0]
    ulays = []
    for ul in range(len(bm.loops.layers.uv)):
        ulays.append(bm.loops.layers.uv[ul])
    writebuf(struct.pack('I', len(ulays)))

    # Verts
    writebuf(struct.pack('I', len(bm.verts)))
    for v in bm.verts:
        writebuf(struct.pack('fff', v.co[0], v.co[1], v.co[2]))
        if dlay:
            sf = tuple(sorted(v[dlay].items()))
            writebuf(struct.pack('I', len(sf)))
            total_len = 0.0
            for ent in sf:
                total_len += ent[1]
            for ent in sf:
                writebuf(struct.pack('If', ent[0], ent[1] / total_len))
        else:
            writebuf(struct.pack('I', 0))

    # Loops
    loop_count = 0
    for f in bm.faces:
        for l in f.loops:
            loop_count += 1
    writebuf(struct.pack('I', loop_count))
    for f in bm.faces:
        for l in f.loops:
            if rna_loops:
                nf = quant_norm(rna_loops[l.index].normal)
            else:
                nf = quant_norm(l.vert.normal)
            writebuf(struct.pack('fff', nf[0], nf[1], nf[2]))
            for cl in range(len(clays)):
                col = l[clays[cl]]
                writebuf(struct.pack('fff', col[0], col[1], col[2]))
            for cl in range(len(ulays)):
                if luvlay and cl == 0 and material_slots[l.face.material_index].material['retro_lightmapped']:
                    uv = quant_luv(l[luvlay].uv)
                else:
                    uv = l[ulays[cl]].uv
                writebuf(struct.pack('ff', uv[0], uv[1]))
            writebuf(struct.pack('IIIII', l.vert.index, l.edge.index, l.face.index,
                                 l.link_loop_next.index, l.link_loop_prev.index))
            if l.edge.is_contiguous:
                writebuf(struct.pack('II', l.link_loop_radial_next.index, l.link_loop_radial_prev.index))
            else:
                writebuf(struct.pack('II', 0xffffffff, 0xffffffff))

    # Edges
    writebuf(struct.pack('I', len(bm.edges)))
    for e in bm.edges:
        for v in e.verts:
            writebuf(struct.pack('I', v.index))
        writebuf(struct.pack('I', len(e.link_faces)))
        for f in e.link_faces:
            writebuf(struct.pack('I', f.index))
        writebuf(struct.pack('b', e.is_contiguous))

    # Faces
    writebuf(struct.pack('I', len(bm.faces)))
    for f in bm.faces:
        norm = f.normal
        writebuf(struct.pack('fff', norm[0], norm[1], norm[2]))
        centroid = f.calc_center_bounds()
        writebuf(struct.pack('fff', centroid[0], centroid[1], centroid[2]))
        writebuf(struct.pack('I', f.material_index))
        for l in f.loops:
            writebuf(struct.pack('I', l.index))

