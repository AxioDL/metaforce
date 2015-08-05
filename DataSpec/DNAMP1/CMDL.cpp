#include <cstddef>
#include "CMDL.hpp"
#include "DNAMP1.hpp"
#include "CMDLMaterials.hpp"

struct DLPrimVert
{
    atUint16 pos;
    atUint16 norm;
    atUint16 uvs[8];
};

namespace Retro
{
namespace DNAMP1
{

bool CMDL::ReadToBlender(HECL::BlenderConnection& conn,
                         Athena::io::IStreamReader& reader,
                         PAKRouter<PAKBridge>& pakRouter,
                         const PAK::Entry& entry,
                         const HECL::ProjectPath& masterShader)
{
    reader.setEndian(Athena::BigEndian);

    CMDL::Header head;
    head.read(reader);

    if (head.magic != 0xDEADBABE)
    {
        Log.report(LogVisor::Error, "invalid CMDL magic");
        return false;
    }

    if (head.version != 2)
    {
        Log.report(LogVisor::Error, "invalid CMDL version for MP1");
        return false;
    }

    /* Open Py Stream */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.hecl_type = 'MESH'\n"
              "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n"
              "\n"
              "# Using 'Blender Game'\n"
              "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
              "\n"
              "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    if ob.type != 'LAMP':\n"
              "        bpy.context.scene.objects.unlink(ob)\n"
              "        bpy.data.objects.remove(ob)\n"
              "\n"
              "# Property to convey original vert indices in overdraw meshes\n"
              "class CMDLOriginalIndex(bpy.types.PropertyGroup):\n"
              "    index = bpy.props.IntProperty(name='Original Vertex Index')\n"
              "bpy.utils.register_class(CMDLOriginalIndex)\n"
              "bpy.types.Mesh.cmdl_orig_verts = bpy.props.CollectionProperty(type=CMDLOriginalIndex)\n"
              "\n"
              "def loop_from_facevert(face, vert_idx):\n"
              "    for loop in face.loops:\n"
              "        if loop.vert.index == vert_idx:\n"
              "            return loop\n"
              "\n"
              "def add_triangle(bm, vert_seq, vert_indices, norm_seq, norm_indices, mat_nr, od_list):\n"
              "    if len(set(vert_indices)) != 3:\n"
              "        return None, None\n"
              "\n"
              "    ret_mesh = bm\n"
              "    vert_seq.ensure_lookup_table()\n"
              "    verts = [vert_seq[i] for i in vert_indices]\n"
              "    norms = [norm_seq[i] for i in norm_indices]\n"
              "\n"
              "    # Make the face\n"
              "    face = bm.faces.get(verts)\n"
              "\n"
              "    if face is not None and face.material_index != mat_nr: # Same poly, new material\n"
              "        # Overdraw detected; track copy\n"
              "        od_entry = None\n"
              "        for entry in od_list:\n"
              "            if entry['material'] == mat_nr:\n"
              "                od_entry = entry\n"
              "        if od_entry is None:\n"
              "            bm_cpy = bm.copy()\n"
              "            od_entry = {'material':mat_nr, 'bm':bm_cpy}\n"
              "            bmesh.ops.delete(od_entry['bm'], geom=od_entry['bm'].faces, context=3)\n"
              "            od_list.append(od_entry)\n"
              "        od_entry['bm'].verts.ensure_lookup_table()\n"
              "        verts = [od_entry['bm'].verts[i] for i in vert_indices]\n"
              "        face = od_entry['bm'].faces.get(verts)\n"
              "        if face is None:\n"
              "            face = od_entry['bm'].faces.new(verts)\n"
              "        else: # Probably a double-sided surface\n"
              "            face = face.copy()\n"
              "            face.normal_flip()\n"
              "        ret_mesh = od_entry['bm']\n"
              "\n"
              "    elif face is not None: # Same material, probably double-sided\n"
              "        face = face.copy()\n"
              "        face.normal_flip()\n"
              "\n"
              "    else: \n"
              "        face = bm.faces.new(verts)\n"
              "\n"
              "    # Apply normals\n"
              "    for i in range(3):\n"
              "        verts[i].normal = norms[i]\n"
              "\n"
              "    for i in range(3):\n"
              "        face.verts[i].index = vert_indices[i]\n"
              "    face.material_index = mat_nr\n"
              "    face.smooth = True\n"
              "\n"
              "    return face, ret_mesh\n"
              "\n"
              "# Begin bmesh\n"
              "bm = bmesh.new()\n"
              "\n", pakRouter.getBestEntryName(entry).c_str());

    /* Link master shader library */
    os.format("# Master shader library\n"
              "with bpy.data.libraries.load('%s', link=True, relative=True) as (data_from, data_to):\n"
              "    data_to.node_groups = data_from.node_groups\n"
              "\n", masterShader.getAbsolutePathUTF8().c_str());

    MaterialSet::RegisterMaterialProps(os);
    os << "# Materials\n"
          "materials = []\n"
          "\n"
          "# Overdraw-tracking\n"
          "od_list = []\n"
          "\n";

    for (size_t s=0 ; s<head.secCount ; ++s)
    {
        atUint64 secStart = reader.position();
        std::vector<std::vector<unsigned>> matUVCounts;
        matUVCounts.reserve(head.matSetCount);
        bool visitedDLOffsets = false;
        if (s < head.matSetCount)
        {
            MaterialSet matSet;
            matSet.read(reader);

            /* Texmaps */
            os << "texmap_list = []\n";
            for (const UniqueID32& tex : matSet.head.textureIDs)
            {
                std::string texName = pakRouter.getBestEntryName(tex);
                HECL::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
                HECL::SystemUTF8View resPathView(resPath);
                os.format("if '%s' in bpy.data.textures:\n"
                          "    image = bpy.data.images['%s']\n"
                          "    texture = bpy.data.textures[image.name]\n"
                          "else:\n"
                          "    image = bpy.data.images.load('//%s')\n"
                          "    image.name = '%s'\n"
                          "    texture = bpy.data.textures.new(image.name, 'IMAGE')\n"
                          "    texture.image = image\n"
                          "texmap_list.append(texture)\n"
                          "\n", texName.c_str(), texName.c_str(),
                          resPathView.str().c_str(), texName.c_str());
            }

            matUVCounts.emplace_back();
            std::vector<unsigned>& uvCounts = matUVCounts.back();
            uvCounts.reserve(matSet.head.materialCount);

            unsigned m=0;
            for (const MaterialSet::Material& mat : matSet.materials)
            {
                uvCounts.emplace_back();
                unsigned& uvCount = uvCounts.back();
                MaterialSet::ConstructMaterial(os, mat, s, m++, uvCount);
                os << "materials.append(new_material)\n";
            }
        }
        else
        {
            switch (s-head.matSetCount)
            {
            case 0:
            {
                /* Positions */
                size_t vertCount = head.secSizes[s] / 12;
                for (size_t i=0 ; i<vertCount ; ++i)
                {
                    atVec3f pos = reader.readVec3f();
                    os.format("bm.verts.new((%f,%f,%f))\n",
                              pos.vec[0], pos.vec[1], pos.vec[2]);
                }
                break;
            }
            case 1:
            {
                /* Normals */
                os << "normals = []\n";
                if (head.flags.shortNormals())
                {
                    size_t normCount = head.secSizes[s] / 6;
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        os.format("normals.append((%f,%f,%f))\n",
                                  reader.readInt16(), reader.readInt16(), reader.readInt16());
                    }
                }
                else
                {
                    size_t normCount = head.secSizes[s] / 12;
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        atVec3f norm = reader.readVec3f();
                        os.format("normals.append((%f,%f,%f))\n",
                                  norm.vec[0], norm.vec[1], norm.vec[2]);
                    }
                }
                break;
            }
            case 2:
            {
                /* Colors */
                break;
            }
            case 3:
            {
                /* Float UVs */
                os << "uv_list = []\n";
                size_t uvCount = head.secSizes[s] / 8;
                for (size_t i=0 ; i<uvCount ; ++i)
                {
                    atVec2f uv = reader.readVec2f();
                    os.format("uv_list.append((%f,%f))\n",
                              uv.vec[0], uv.vec[1]);
                }
                break;
            }
            case 4:
            {
                /* Short UVs */
                os << "suv_list = []\n";
                if (head.flags.shortUVs())
                {
                    size_t uvCount = head.secSizes[s] / 4;
                    for (size_t i=0 ; i<uvCount ; ++i)
                    {
                        os.format("suv_list.append((%f,%f))\n",
                                  reader.readInt16(), reader.readInt16());
                    }
                    break;
                }

                /* DL Offsets (here or next section) */
                visitedDLOffsets = true;
                break;
            }
            default:
            {
                if (!visitedDLOffsets)
                {
                    visitedDLOffsets = true;
                    break;
                }

                /* GX Display List (surface) */
                SurfaceHeader sHead;
                sHead.read(reader);
                unsigned matUVCount = matUVCounts[0][sHead.matIdx];

                std::unique_ptr<atUint8[]> dlBuf = reader.readUBytes(sHead.dlSize);
                atUint8* origDl = dlBuf.get();
                atUint8* dl = origDl;

                while (*dl && (dl-origDl) < sHead.dlSize)
                {

                    GX::Primitive ptype = GX::Primitive(*dl);
                    atUint16 vert_count = HECL::SBig(*(atUint16*)(dl + 1));
                    os.format("# VAT Type: %u\n", (*(atUint8*)dl)&7);

                    atUint16* dli = (atUint16*)(dl + 3);

                    /* First vert */
                    DLPrimVert first_prim_vert;
                    first_prim_vert.pos = HECL::SBig(dli[0]);
                    first_prim_vert.norm = HECL::SBig(dli[1]);
                    for (int uv=0 ; uv<matUVCount ; ++uv)
                        first_prim_vert.uvs[uv] = HECL::SBig(dli[2+uv]);

                    /* 3 Prim Verts to start */
                    int c = 0;
                    DLPrimVert prim_verts[3];
                    for (int pv=0 ; pv<3 ; ++pv)
                    {
                        prim_verts[pv].pos = HECL::SBig(*dli++);
                        prim_verts[pv].norm = HECL::SBig(*dli++);
                        for (int uv=0 ; uv<matUVCount ; ++uv)
                            prim_verts[pv].uvs[uv] = HECL::SBig(*dli++);
                    }

                    if (ptype == GX::TRIANGLESTRIP)
                    {
                        atUint8 flip = 0;
                        for (int v=0 ; v<vert_count-2 ; ++v)
                        {

                            if (flip)
                            {
                                os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                          prim_verts[c%3].pos,
                                          prim_verts[(c+2)%3].pos,
                                          prim_verts[(c+1)%3].pos,
                                          prim_verts[c%3].norm,
                                          prim_verts[(c+2)%3].norm,
                                          prim_verts[(c+1)%3].norm,
                                          sHead.matIdx);
                                if (matUVCount)
                                {
                                    os << "if last_face is not None:\n";
                                    for (int j=0 ; j<matUVCount ; ++j)
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  prim_verts[c%3].pos, j, prim_verts[c%3].uvs[j],
                                                  prim_verts[(c+2)%3].pos, j, prim_verts[(c+2)%3].uvs[j],
                                                  prim_verts[(c+1)%3].pos, j, prim_verts[(c+1)%3].uvs[j]);
                                }
                            }
                            else
                            {
                                os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                          prim_verts[c%3].pos,
                                          prim_verts[(c+1)%3].pos,
                                          prim_verts[(c+2)%3].pos,
                                          prim_verts[c%3].norm,
                                          prim_verts[(c+1)%3].norm,
                                          prim_verts[(c+2)%3].norm,
                                          sHead.matIdx);
                                if (matUVCount)
                                {
                                    os << "if last_face is not None:\n";
                                    for (int j=0 ; j<matUVCount ; ++j)
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  prim_verts[c%3].pos, j, prim_verts[c%3].uvs[j],
                                                  prim_verts[(c+1)%3].pos, j, prim_verts[(c+1)%3].uvs[j],
                                                  prim_verts[(c+2)%3].pos, j, prim_verts[(c+2)%3].uvs[j]);
                                }
                            }
                            flip ^= 1;

                            dl = (atUint8*)dli;

                            /* Advance one prim vert */
                            prim_verts[c%3].pos = HECL::SBig(*dli++);
                            prim_verts[c%3].norm = HECL::SBig(*dli++);
                            for (int uv=0 ; uv<matUVCount ; ++uv)
                                prim_verts[c%3].uvs[uv] = HECL::SBig(*dli++);
                            ++c;

                        }
                    }
                    else if (ptype == GX::TRIANGLES)
                    {
                        for (int v=0 ; v<vert_count ; v+=3)
                        {

                            os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                      prim_verts[0].pos,
                                      prim_verts[1].pos,
                                      prim_verts[2].pos,
                                      prim_verts[0].norm,
                                      prim_verts[1].norm,
                                      prim_verts[2].norm,
                                      sHead.matIdx);
                            if (matUVCount)
                            {
                                os << "if last_face is not None:\n";
                                for (int j=0 ; j<matUVCount ; ++j)
                                    os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                              "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                              "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                              prim_verts[0].pos, j, prim_verts[0].uvs[j],
                                              prim_verts[1].pos, j, prim_verts[1].uvs[j],
                                              prim_verts[2].pos, j, prim_verts[2].uvs[j]);
                            }

                            dl = (atUint8*)dli;

                            /* Break if done */
                            if (v+3 >= vert_count)
                                break;

                            /* Advance 3 Prim Verts */
                            for (int pv=0 ; pv<3 ; ++pv)
                            {
                                prim_verts[pv].pos = HECL::SBig(*dli++);
                                prim_verts[pv].norm = HECL::SBig(*dli++);
                                for (int uv=0 ; uv<matUVCount ; ++uv)
                                    prim_verts[pv].uvs[uv] = HECL::SBig(*dli++);
                            }
                        }
                    }
                    else if (ptype == GX::TRIANGLEFAN)
                    {
                        ++c;
                        for (int v=0 ; v<vert_count-2 ; ++v)
                        {
                            os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                      first_prim_vert.pos,
                                      prim_verts[c%3].pos,
                                      prim_verts[(c+1)%3].pos,
                                      first_prim_vert.norm,
                                      prim_verts[c%3].norm,
                                      prim_verts[(c+1)%3].norm,
                                      sHead.matIdx);
                            if (matUVCount)
                            {
                                os << "if last_face is not None:\n";
                                for (int j=0 ; j<matUVCount ; ++j)
                                    os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                              "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                              "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                              first_prim_vert.pos, j, first_prim_vert.uvs[j],
                                              prim_verts[c%3].pos, j, prim_verts[c%3].uvs[j],
                                              prim_verts[(c+1)%3].pos, j, prim_verts[(c+1)%3].uvs[j]);
                            }

                            dl = (atUint8*)dli;

                            /* Advance one prim vert */
                            prim_verts[(c+2)%3].pos = HECL::SBig(*dli++);
                            prim_verts[(c+2)%3].norm = HECL::SBig(*dli++);
                            for (int uv=0 ; uv<matUVCount ; ++uv)
                                prim_verts[(c+2)%3].uvs[uv] = HECL::SBig(*dli++);
                            ++c;
                        }
                    }
                    os << "\n";
                }
            }
            }
        }

        if (s < head.secCount - 1)
            reader.seek(secStart + head.secSizes[s], Athena::Begin);
    }

    /* Finish Mesh */
    os.format("mesh = bpy.data.meshes.new(bpy.context.scene.name)\n"
              "obj = bpy.data.objects.new(mesh.name, mesh)\n"
              "obj.show_transparent = True\n"
              "bpy.context.scene.objects.link(obj)\n"
              "mesh.cmdl_material_count = %u\n"
              "for material in materials:\n"
              "    mesh.materials.append(material)\n"
              "\n"
              "# Preserve original indices\n"
              "for vert in bm.verts:\n"
              "    ov = mesh.cmdl_orig_verts.add()\n"
              "    ov.index = vert.index\n"
              "\n"
              "# Merge OD meshes\n"
              "for od_entry in od_list:\n"
              "    vert_dict = {}\n"
              "\n"
              "    for vert in od_entry['bm'].verts:\n"
              "        if len(vert.link_faces):\n"
              "            vert_dict[vert.index] = bm.verts.new(vert.co, vert)\n"
              "            ov = mesh.cmdl_orig_verts.add()\n"
              "            ov.index = vert.index\n"
              "\n"
              "    for face in od_entry['bm'].faces:\n"
              "        merge_verts = [vert_dict[fv.index] for fv in face.verts]\n"
              "        if bm.faces.get(merge_verts) is not None:\n"
              "            continue\n"
              "        merge_face = bm.faces.new(merge_verts)\n"
              "        for i in range(len(face.loops)):\n"
              "            old = face.loops[i]\n"
              "            new = merge_face.loops[i]\n"
              "            for j in range(len(od_entry['bm'].loops.layers.uv)):\n"
              "                new[bm.loops.layers.uv[j]] = old[od_entry['bm'].loops.layers.uv[j]]\n"
              "        merge_face.smooth = True\n"
              "        merge_face.material_index = face.material_index\n"
              "\n"
              "    od_entry['bm'].free()\n"
              "\n"
              "# Remove loose vertices\n"
              "#to_remove = []\n"
              "#for vert in bm.verts:\n"
              "#    if not len(vert.link_faces):\n"
              "#        to_remove.append(vert)\n"
              "#bmesh.ops.delete(bm, geom=to_remove, context=1)\n"
              "\n"
              "bm.to_mesh(mesh)\n"
              "bm.free()\n"
              "\n", head.matSetCount);

    return true;
}

}
}
