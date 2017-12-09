#include "DeafBabe.hpp"
#include "AROTBuilder.hpp"
#include "DataSpec/DNAMP1/DeafBabe.hpp"
#include "DataSpec/DNAMP2/DeafBabe.hpp"
#include "DataSpec/DNAMP1/DCLN.hpp"
#include <inttypes.h>

namespace DataSpec
{

template<class DEAFBABE>
void DeafBabeSendToBlender(hecl::BlenderConnection::PyOutStream& os, const DEAFBABE& db, bool isDcln, atInt32 idx)
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

        int vindices[3];
        vindices[2] =
            (edge1.verts[0] != edge0.verts[0] && edge1.verts[0] != edge0.verts[1]) ?
            edge1.verts[0] : edge1.verts[1];

        if (triMat.flipFace())
        {
            vindices[0] = edge0.verts[1];
            vindices[1] = edge0.verts[0];
        }
        else
        {
            vindices[0] = edge0.verts[0];
            vindices[1] = edge0.verts[1];
        }

        os << "tri_verts = []\n";
        os.format("tri_verts.append(col_bm.verts[%u])\n", vindices[0]);
        os.format("tri_verts.append(col_bm.verts[%u])\n", vindices[1]);
        os.format("tri_verts.append(col_bm.verts[%u])\n", vindices[2]);

        os.format("face = col_bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = col_bm.faces.new(tri_verts)\n"
                  "else:\n"
                  "    face = face.copy()\n"
                  "    for i in range(3):\n"
                  "        face.verts[i].co = tri_verts[i].co\n"
                  "    col_bm.verts.ensure_lookup_table()\n"
                  "face.material_index = select_material(0x%016" PRIX64 ")\n"
                  "face.smooth = False\n"
                  "\n",
                  atUint64(triMat.material));
    }

    db.insertNoClimb(os);

    if (isDcln)
        os.format("col_mesh = bpy.data.meshes.new('CMESH_%i')\n", idx);
    else
        os << "col_mesh = bpy.data.meshes.new('CMESH')\n";

    os << "col_bm.to_mesh(col_mesh)\n"
          "col_mesh_obj = bpy.data.objects.new(col_mesh.name, col_mesh)\n"
          "\n"
          "for mat_name in material_index:\n"
          "    mat = material_dict[mat_name]\n"
          "    col_mesh.materials.append(mat)\n"
          "\n"
          "bpy.context.scene.objects.link(col_mesh_obj)\n"
          "bpy.context.scene.objects.active = col_mesh_obj\n"
          "bpy.ops.object.mode_set(mode='EDIT')\n"
          "bpy.ops.mesh.tris_convert_to_quads()\n"
          "bpy.ops.object.mode_set(mode='OBJECT')\n"
          "bpy.context.scene.objects.active = None\n";
    if (!isDcln)
        os << "col_mesh_obj.layers[1] = True\n"
              "col_mesh_obj.layers[0] = False\n";


    os << "col_mesh_obj.draw_type = 'SOLID'\n"
          "col_mesh_obj.game.physics_type = 'STATIC'\n"
          "\n";
}

template void DeafBabeSendToBlender<DNAMP1::DeafBabe>(hecl::BlenderConnection::PyOutStream& os, const DNAMP1::DeafBabe& db, bool isDcln, atInt32 idx);
template void DeafBabeSendToBlender<DNAMP2::DeafBabe>(hecl::BlenderConnection::PyOutStream& os, const DNAMP2::DeafBabe& db, bool isDcln, atInt32 idx);
template void DeafBabeSendToBlender<DNAMP1::DCLN::Collision>(hecl::BlenderConnection::PyOutStream& os, const DNAMP1::DCLN::Collision& db, bool isDcln, atInt32 idx);

template<class DEAFBABE>
static void PopulateAreaFields(DEAFBABE& db,
    const hecl::BlenderConnection::DataStream::ColMesh& colMesh,
    const zeus::CAABox& fullAABB,
    std::enable_if_t<std::is_same<DEAFBABE, DNAMP1::DeafBabe>::value ||
                     std::is_same<DEAFBABE, DNAMP2::DeafBabe>::value, int>* = 0)
{
    AROTBuilder builder;
    auto octree = builder.buildCol(colMesh, db.rootNodeType);
    static_cast<std::unique_ptr<atUint8[]>&>(db.bspTree) = std::move(octree.first);
    db.bspSize = octree.second;

    db.unk1 = 0x1000000;
    db.length = db.binarySize(0) - 8;
    db.magic = 0xDEAFBABE;
    db.version = 3;
    db.aabb[0] = fullAABB.min;
    db.aabb[1] = fullAABB.max;
}

template<class DEAFBABE>
static void PopulateAreaFields(DEAFBABE& db,
    const hecl::BlenderConnection::DataStream::ColMesh& colMesh,
    const zeus::CAABox& fullAABB,
    std::enable_if_t<std::is_same<DEAFBABE, DNAMP1::DCLN::Collision>::value, int>* = 0)
{
    db.magic = 0xDEAFBABE;
    db.version = 2;
    db.memSize = 0;
}

template<class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh)
{
    db.materials.reserve(colMesh.materials.size() * 2);
    for (const hecl::BlenderConnection::DataStream::ColMesh::Material& mat : colMesh.materials)
    {
        db.materials.emplace_back();
        db.materials.back().setUnknown(mat.unknown);
        db.materials.back().setSurfaceStone(mat.surfaceStone);
        db.materials.back().setSurfaceMetal(mat.surfaceMetal);
        db.materials.back().setSurfaceGrass(mat.surfaceGrass);
        db.materials.back().setSurfaceIce(mat.surfaceIce);
        db.materials.back().setPillar(mat.pillar);
        db.materials.back().setSurfaceMetalGrating(mat.surfaceMetalGrating);
        db.materials.back().setSurfacePhazon(mat.surfacePhazon);
        db.materials.back().setSurfaceDirt(mat.surfaceDirt);
        db.materials.back().setSurfaceLava(mat.surfaceLava);
        db.materials.back().setSurfaceSPMetal(mat.surfaceSPMetal);
        db.materials.back().setSurfaceStoneRock(mat.surfaceStoneRock);
        db.materials.back().setSurfaceSnow(mat.surfaceSnow);
        db.materials.back().setSurfaceMudSlow(mat.surfaceMudSlow);
        db.materials.back().setSurfaceFabric(mat.surfaceFabric);
        db.materials.back().setHalfPipe(mat.halfPipe);
        db.materials.back().setSurfaceMud(mat.surfaceMud);
        db.materials.back().setSurfaceGlass(mat.surfaceGlass);
        db.materials.back().setUnused3(mat.unused3);
        db.materials.back().setUnused4(mat.unused4);
        db.materials.back().setSurfaceShield(mat.surfaceShield);
        db.materials.back().setSurfaceSand(mat.surfaceSand);
        db.materials.back().setSurfaceMothOrSeedOrganics(mat.surfaceMothOrSeedOrganics);
        db.materials.back().setSurfaceWeb(mat.surfaceWeb);
        db.materials.back().setProjectilePassthrough(mat.projPassthrough);
        db.materials.back().setSolid(mat.solid);
        db.materials.back().setU20(mat.u20);
        db.materials.back().setCameraPassthrough(mat.camPassthrough);
        db.materials.back().setSurfaceWood(mat.surfaceWood);
        db.materials.back().setSurfaceOrganic(mat.surfaceOrganic);
        db.materials.back().setU24(mat.u24);
        db.materials.back().setSurfaceRubber(mat.surfaceRubber);
        db.materials.back().setSeeThrough(mat.seeThrough);
        db.materials.back().setScanPassthrough(mat.scanPassthrough);
        db.materials.back().setAiPassthrough(mat.aiPassthrough);
        db.materials.back().setCeiling(mat.ceiling);
        db.materials.back().setWall(mat.wall);
        db.materials.back().setFloor(mat.floor);
        db.materials.back().setAiBlock(mat.aiBlock);
        db.materials.back().setJumpNotAllowed(mat.jumpNotAllowed);
        db.materials.back().setSpiderBall(mat.spiderBall);
        db.materials.back().setScrewAttackWallJump(mat.screwAttackWallJump);
    }

    zeus::CAABox fullAABB;

    db.verts.reserve(colMesh.verts.size());
    db.vertMats.resize(colMesh.verts.size());
    for (const auto& vert : colMesh.verts)
    {
        fullAABB.accumulateBounds(zeus::CVector3f(vert));
        db.verts.push_back(vert);
    }
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
        if (tri.flip)
        {
            db.triMats.push_back(db.materials.size());
            db.materials.push_back(db.materials[tri.matIdx]);
            db.materials.back().setFlipFace(true);
        }
        else
        {
            db.triMats.push_back(tri.matIdx);
        }

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

    db.materialCount = db.materials.size();

    PopulateAreaFields(db, colMesh, fullAABB);
}

template void DeafBabeBuildFromBlender<DNAMP1::DeafBabe>(DNAMP1::DeafBabe& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);
template void DeafBabeBuildFromBlender<DNAMP2::DeafBabe>(DNAMP2::DeafBabe& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);
template void DeafBabeBuildFromBlender<DNAMP1::DCLN::Collision>(DNAMP1::DCLN::Collision& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);

}
