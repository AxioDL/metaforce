#include "DeafBabe.hpp"
#include "AROTBuilder.hpp"
#include "DataSpec/DNAMP1/DeafBabe.hpp"
#include "DataSpec/DNAMP2/DeafBabe.hpp"

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

template void DeafBabeSendToBlender<DNAMP1::DeafBabe>(hecl::BlenderConnection::PyOutStream& os, const DNAMP1::DeafBabe& db);
template void DeafBabeSendToBlender<DNAMP2::DeafBabe>(hecl::BlenderConnection::PyOutStream& os, const DNAMP2::DeafBabe& db);

template<class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh)
{
    {
        AROTBuilder builder;
        auto octree = builder.buildCol(colMesh, db.rootNodeType);
        static_cast<std::unique_ptr<atUint8[]>&>(db.bspTree) = std::move(octree.first);
        db.bspSize = octree.second;
    }

    db.materials.reserve(colMesh.materials.size());
    for (const auto& mat : colMesh.materials)
    {
        db.materials.emplace_back();
        db.materials.back().setFireThrough(mat.fireThrough);
        db.materials.back().setType(typename DEAFBABE::Material::Type(mat.type));
    }
    db.materialCount = colMesh.materials.size();

    db.verts.reserve(colMesh.verts.size());
    db.vertMats.resize(colMesh.verts.size());
    for (const auto& vert : colMesh.verts)
        db.verts.push_back(vert);
    db.vertMatsCount = colMesh.verts.size();
    db.vertCount = colMesh.verts.size();

    db.edgeVertConnections.reserve(colMesh.edges.size());
    db.edgeMats.resize(colMesh.edges.size());
    for (const auto& edge : colMesh.edges)
    {
        db.edgeVertConnections.emplace_back();
        db.edgeVertConnections.back().verts[0] = edge.verts[0];
        db.edgeVertConnections.back().verts[1] = edge.verts[1];
    }
    db.edgeMatsCount = colMesh.edges.size();
    db.edgeVertsCount = colMesh.edges.size();

    db.triMats.reserve(colMesh.trianges.size());
    db.triangleEdgeConnections.reserve(colMesh.trianges.size());
    for (const auto& tri : colMesh.trianges)
    {
        db.triMats.push_back(tri.matIdx);

        db.triangleEdgeConnections.emplace_back();
        db.triangleEdgeConnections.back().edges[0] = tri.edges[0];
        db.triangleEdgeConnections.back().edges[1] = tri.edges[1];
        db.triangleEdgeConnections.back().edges[2] = tri.edges[2];

        for (int e=0 ; e<3 ; ++e)
        {
            db.edgeMats[tri.edges[e]] = tri.matIdx;
            for (int v=0 ; v<2 ; ++v)
                db.vertMats[colMesh.edges[e].verts[v]] = tri.matIdx;
        }
    }
    db.triMatsCount = colMesh.trianges.size();
    db.triangleEdgesCount = colMesh.trianges.size() * 3;

    db.unk1 = 0;
    db.length = db.binarySize(0) - 8;
    db.magic = 0xDEAFBABE;
    db.version = 0;
    db.aabb[0] = colMesh.aabbMin;
    db.aabb[1] = colMesh.aabbMax;
}

template void DeafBabeBuildFromBlender<DNAMP1::DeafBabe>(DNAMP1::DeafBabe& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);
template void DeafBabeBuildFromBlender<DNAMP2::DeafBabe>(DNAMP2::DeafBabe& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);

}
