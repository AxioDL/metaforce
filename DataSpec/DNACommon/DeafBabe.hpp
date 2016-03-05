#ifndef _DNACOMMON_DEAFBABE_HPP_
#define _DNACOMMON_DEAFBABE_HPP_

#include "DNACommon.hpp"
#include "BlenderConnection.hpp"

namespace DataSpec
{

template<class DEAFBABE>
void DeafBabeSendToBlender(hecl::BlenderConnection::PyOutStream& os, const DEAFBABE& db)
{
    os << "material_index = []\n"
          "col_bm = bmesh.new()\n";
    for (const atVec3f& vert : db.verts)
        os.format("col_bm.verts.new((%f,%f,%f))\n",
                  vert.vec[0],
                  vert.vec[1],
                  vert.vec[2]);

    os << "col_bm.verts.ensure_lookup_table()\n";

    int triIdx = 0;
    for (const typename DEAFBABE::Triangle& tri : db.triangleEdgeConnections)
    {
        const typename DEAFBABE::Material& triMat = db.materials[db.triMats[triIdx++]];
        const typename DEAFBABE::Edge& edge0 = db.edgeVertConnections[tri.edges[0]];
        const typename DEAFBABE::Edge& edge1 = db.edgeVertConnections[tri.edges[1]];
        const typename DEAFBABE::Edge& edge2 = db.edgeVertConnections[tri.edges[2]];
        if (!edge0.verts[0] && !edge1.verts[0] && !edge2.verts[0])
            break;
        os << "tri_verts = []\n";
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge0.findCommon(edge1));
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge1.findCommon(edge2));
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge2.findCommon(edge0));

        os.format("face = col_bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = col_bm.faces.new(tri_verts)\n"
                  "else:\n"
                  "    face = face.copy()\n"
                  "    for i in range(3):\n"
                  "        face.verts[i].co = tri_verts[i].co\n"
                  "    col_bm.verts.ensure_lookup_table()\n"
                  "face.material_index = select_material(0x%08X)\n"
                  "face.smooth = False\n"
                  "\n",
                  atUint32(triMat.material));
    }

    db.insertNoClimb(os);

    os << "col_mesh = bpy.data.meshes.new('CMESH')\n"
          "col_bm.to_mesh(col_mesh)\n"
          "col_mesh_obj = bpy.data.objects.new(col_mesh.name, col_mesh)\n"
          "\n"
          "for mat_name in material_index:\n"
          "    mat = material_dict[mat_name]\n"
          "    col_mesh.materials.append(mat)\n"
          "\n"
          "bpy.context.scene.objects.link(col_mesh_obj)\n"
          "col_mesh_obj.layers[1] = True\n"
          "col_mesh_obj.layers[0] = False\n"
          "col_mesh_obj.draw_type = 'SOLID'\n"
          "col_mesh_obj.game.physics_type = 'STATIC'\n"
          "\n";
}

}

#endif // _DNACOMMON_DEAFBABE_HPP_
