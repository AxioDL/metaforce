#ifndef __DNACOMMON_MAPA_HPP__
#define __DNACOMMON_MAPA_HPP__

#include "DNACommon.hpp"
#include "GX.hpp"

namespace Retro
{
namespace DNAMAPA
{

template <typename PAKRouter, typename MAPA>
bool ReadMAPAToBlender(HECL::BlenderConnection& conn,
                       const MAPA& mapa,
                       const HECL::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force)
{
    /* Rename MAPA for consistency */
    HECL::ProjectPath mapaPath(outPath.getParentPath(), _S("!map.blend"));
    if (!force && mapaPath.getPathType() == HECL::ProjectPath::PT_FILE)
        return true;

    if (!conn.createBlend(mapaPath, HECL::BlenderConnection::TypeMapArea))
        return false;
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os << "import bpy, bmesh\n"
          "from mathutils import Matrix\n"
          "\n"
          "bpy.types.Object.retro_mappable_type = bpy.props.IntProperty(name='Retro: MAPA object type', default=-1)\n"
          "bpy.types.Object.retro_mappable_unk = bpy.props.IntProperty(name='Retro: MAPA object unk')\n"
          "bpy.types.Object.retro_mappable_sclyid = bpy.props.StringProperty(name='Retro: MAPA object SCLY ID')\n"
          "\n"
          "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    bpy.context.scene.objects.unlink(ob)\n"
          "    bpy.data.objects.remove(ob)\n"
          "\n"
          "def add_triangle(bm, verts):\n"
          "    verts = [bm.verts[vi] for vi in verts]\n"
          "    face = bm.faces.get(verts)\n"
          "    if face:\n"
          "        face = face.copy()\n"
          "        bm.verts.ensure_lookup_table()\n"
          "        face.normal_flip()\n"
          "    else:\n"
          "        bm.faces.new(verts)\n"
          "\n"
          "def add_border(bm, verts):\n"
          "    verts = [bm.verts[vi] for vi in verts]\n"
          "    edge = bm.edges.get(verts)\n"
          "    edge.seam = True\n"
          "\n";

    os.format("bpy.context.scene.name = 'MAPA_%s'\n",
              entry.id.toString().c_str());

    /* Add empties representing MappableObjects */
    int moIdx = 0;
    for (const typename MAPA::MappableObject& mo : mapa.mappableObjects)
    {
        os.format("obj = bpy.data.objects.new('MAPOBJ_%02d', None)\n"
                  "bpy.context.scene.objects.link(obj)\n"
                  "obj.retro_mappable_type = %d\n"
                  "obj.retro_mappable_unk = %d\n"
                  "obj.retro_mappable_sclyid = '%08X'\n"
                  "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "obj.rotation_mode = 'QUATERNION'\n"
                  "obj.location = mtxd[0]\n"
                  "obj.rotation_quaternion = mtxd[1]\n"
                  "obj.scale = mtxd[2]\n",
                  moIdx, mo.type, mo.unknown1, mo.sclyId,
                  mo.transformMtx[0].vec[0], mo.transformMtx[0].vec[1], mo.transformMtx[0].vec[2], mo.transformMtx[0].vec[3],
                  mo.transformMtx[1].vec[0], mo.transformMtx[1].vec[1], mo.transformMtx[1].vec[2], mo.transformMtx[1].vec[3],
                  mo.transformMtx[2].vec[0], mo.transformMtx[2].vec[1], mo.transformMtx[2].vec[2], mo.transformMtx[2].vec[3]);
        ++moIdx;
    }

    os << "# Begin bmesh\n"
          "bm = bmesh.new()\n"
          "\n";

    /* Read in verts */
    for (const atVec3f& vert : mapa.vertices)
        os.format("bm.verts.new((%f,%f,%f))\n",
                  vert.vec[0], vert.vec[1], vert.vec[2]);
    os << "bm.verts.ensure_lookup_table()\n";

    /* Read in surfaces */
    for (const typename MAPA::Surface& surf : mapa.surfaces)
    {
        for (const typename MAPA::Surface::Primitive& prim : surf.primitives)
        {
            auto iit = prim.indices.cbegin();

            /* 3 Prim Verts to start */
            int c = 0;
            unsigned int primVerts[3] =
            {
                *iit++,
                *iit++,
                *iit++
            };

            if (GX::Primitive(prim.type) == GX::TRIANGLESTRIP)
            {
                atUint8 flip = 0;
                for (int v=0 ; v<prim.indexCount-2 ; ++v)
                {
                    if (flip)
                    {
                        os.format("add_triangle(bm, (%u,%u,%u))\n",
                                  primVerts[c%3],
                                  primVerts[(c+2)%3],
                                  primVerts[(c+1)%3]);
                    }
                    else
                    {
                        os.format("add_triangle(bm, (%u,%u,%u))\n",
                                  primVerts[c%3],
                                  primVerts[(c+1)%3],
                                  primVerts[(c+2)%3]);
                    }
                    flip ^= 1;

                    bool peek = (v >= prim.indexCount - 3);

                    /* Advance one prim vert */
                    if (peek)
                        primVerts[c%3] = *iit;
                    else
                        primVerts[c%3] = *iit++;
                    ++c;
                }
            }
            else if (GX::Primitive(prim.type) == GX::TRIANGLES)
            {
                for (int v=0 ; v<prim.indexCount ; v+=3)
                {
                    os.format("add_triangle(bm, (%u,%u,%u))\n",
                              primVerts[0],
                              primVerts[1],
                              primVerts[2]);

                    /* Break if done */
                    if (v+3 >= prim.indexCount)
                        break;

                    /* Advance 3 Prim Verts */
                    for (int pv=0 ; pv<3 ; ++pv)
                        primVerts[pv] = *iit++;
                }
            }
        }

        for (const typename MAPA::Surface::Border& border : surf.borders)
        {
            auto iit = border.indices.cbegin();
            for (int i=0 ; i<border.indexCount-1 ; ++i)
            {
                os.format("add_border(bm, (%u,%u))\n",
                          *iit, *(iit+1));
                ++iit;
            }
        }
    }


    os << "mesh = bpy.data.meshes.new('MAP')\n"
          "obj = bpy.data.objects.new(mesh.name, mesh)\n"
          "bm.to_mesh(mesh)\n"
          "bpy.context.scene.objects.link(obj)\n"
          "bm.free()\n";

    /* World background */
    HECL::ProjectPath worldBlend(outPath.getParentPath().getParentPath(), "!world.blend");
    if (worldBlend.getPathType() == HECL::ProjectPath::PT_FILE)
        os.linkBackground("//../!world.blend", "World");

    os.centerView();
    os.close();
    conn.saveBlend();
    return true;
}

}
}

#endif // __DNACOMMON_MAPA_HPP__
