#include "DeafBabe.hpp"
#include "AROTBuilder.hpp"
#include "DataSpec/DNAMP1/DeafBabe.hpp"
#include "DataSpec/DNAMP2/DeafBabe.hpp"
#include "DataSpec/DNAMP1/DCLN.hpp"
#include "hecl/Blender/Connection.hpp"
#include <cinttypes>

namespace DataSpec {

template <class DEAFBABE>
void DeafBabeSendToBlender(hecl::blender::PyOutStream& os, const DEAFBABE& db, bool isDcln, atInt32 idx) {
  os << "material_index = []\n"
        "col_bm = bmesh.new()\n";
  for (const atVec3f& vert : db.verts) {
    zeus::simd_floats f(vert.simd);
    os.format(fmt("col_bm.verts.new(({},{},{}))\n"), f[0], f[1], f[2]);
  }

  os << "col_bm.verts.ensure_lookup_table()\n";

  int triIdx = 0;
  for (const typename DEAFBABE::Triangle& tri : db.triangleEdgeConnections) {
    const typename DEAFBABE::Material& triMat = db.materials[db.triMats[triIdx++]];
    const typename DEAFBABE::Edge& edge0 = db.edgeVertConnections[tri.edges[0]];
    const typename DEAFBABE::Edge& edge1 = db.edgeVertConnections[tri.edges[1]];
    const typename DEAFBABE::Edge& edge2 = db.edgeVertConnections[tri.edges[2]];
    if (!edge0.verts[0] && !edge1.verts[0] && !edge2.verts[0])
      break;

    int vindices[3];
    vindices[2] =
        (edge1.verts[0] != edge0.verts[0] && edge1.verts[0] != edge0.verts[1]) ? edge1.verts[0] : edge1.verts[1];

    if (triMat.flipFace()) {
      vindices[0] = edge0.verts[1];
      vindices[1] = edge0.verts[0];
    } else {
      vindices[0] = edge0.verts[0];
      vindices[1] = edge0.verts[1];
    }

    os << "tri_verts = []\n";
    os.format(fmt("tri_verts.append(col_bm.verts[{}])\n"), vindices[0]);
    os.format(fmt("tri_verts.append(col_bm.verts[{}])\n"), vindices[1]);
    os.format(fmt("tri_verts.append(col_bm.verts[{}])\n"), vindices[2]);

    os.format(fmt(
        "face = col_bm.faces.get(tri_verts)\n"
        "if face is None:\n"
        "    face = col_bm.faces.new(tri_verts)\n"
        "else:\n"
        "    face = face.copy()\n"
        "    for i in range(3):\n"
        "        face.verts[i].co = tri_verts[i].co\n"
        "    col_bm.verts.ensure_lookup_table()\n"
        "face.material_index = select_material(0x{:016X}"
        ")\n"
        "face.smooth = False\n"
        "\n"),
        atUint64(triMat.material));
  }

  db.insertNoClimb(os);

  if (isDcln)
    os.format(fmt("col_mesh = bpy.data.meshes.new('CMESH_{}')\n"), idx);
  else
    os << "col_mesh = bpy.data.meshes.new('CMESH')\n";

  os << "col_bm.to_mesh(col_mesh)\n"
        "col_mesh_obj = bpy.data.objects.new(col_mesh.name, col_mesh)\n"
        "\n"
        "for mat_name in material_index:\n"
        "    mat = material_dict[mat_name]\n"
        "    col_mesh.materials.append(mat)\n"
        "\n"
        "if 'Collision' not in bpy.data.collections:\n"
        "    coll = bpy.data.collections.new('Collision')\n"
        "    bpy.context.scene.collection.children.link(coll)\n"
        "else:\n"
        "    coll = bpy.data.collections['Collision']\n"
        "coll.objects.link(col_mesh_obj)\n"
        "bpy.context.view_layer.objects.active = col_mesh_obj\n"
        "bpy.ops.object.mode_set(mode='EDIT')\n"
        "bpy.ops.mesh.tris_convert_to_quads()\n"
        "bpy.ops.object.mode_set(mode='OBJECT')\n"
        "bpy.context.view_layer.objects.active = None\n"
        "col_mesh_obj.display_type = 'SOLID'\n"
        "\n";
}

template void DeafBabeSendToBlender<DNAMP1::DeafBabe>(hecl::blender::PyOutStream& os, const DNAMP1::DeafBabe& db,
                                                      bool isDcln, atInt32 idx);
template void DeafBabeSendToBlender<DNAMP2::DeafBabe>(hecl::blender::PyOutStream& os, const DNAMP2::DeafBabe& db,
                                                      bool isDcln, atInt32 idx);
template void DeafBabeSendToBlender<DNAMP1::DCLN::Collision>(hecl::blender::PyOutStream& os,
                                                             const DNAMP1::DCLN::Collision& db, bool isDcln,
                                                             atInt32 idx);

template <class DEAFBABE>
static void PopulateAreaFields(
    DEAFBABE& db, const hecl::blender::ColMesh& colMesh, const zeus::CAABox& fullAABB,
    std::enable_if_t<std::is_same<DEAFBABE, DNAMP1::DeafBabe>::value || std::is_same<DEAFBABE, DNAMP2::DeafBabe>::value,
                     int>* = 0) {
  AROTBuilder builder;
  auto octree = builder.buildCol(colMesh, db.rootNodeType);
  static_cast<std::unique_ptr<atUint8[]>&>(db.bspTree) = std::move(octree.first);
  db.bspSize = octree.second;

  db.unk1 = 0x1000000;
  size_t dbSize = 0;
  db.binarySize(dbSize);
  db.length = dbSize - 8;
  db.magic = 0xDEAFBABE;
  db.version = 3;
  db.aabb[0] = fullAABB.min;
  db.aabb[1] = fullAABB.max;
}

template <class DEAFBABE>
static void PopulateAreaFields(DEAFBABE& db, const hecl::blender::ColMesh& colMesh, const zeus::CAABox& fullAABB,
                               std::enable_if_t<std::is_same<DEAFBABE, DNAMP1::DCLN::Collision>::value, int>* = 0) {
  db.magic = 0xDEAFBABE;
  db.version = 2;
  db.memSize = 0;
}

class MaterialPool {
  std::unordered_map<u64, int> m_materials;

public:
  template <class M, class V>
  int AddOrLookup(const M& mat, V& vec) {
    auto search = m_materials.find(mat.material);
    if (search != m_materials.end())
      return search->second;
    auto idx = int(vec.size());
    vec.push_back(mat);
    m_materials[mat.material] = idx;
    return idx;
  }
};

template <class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::blender::ColMesh& colMesh) {
  using BlendMat = hecl::blender::ColMesh::Material;

  auto MakeMat = [](const BlendMat& mat, bool flipFace) -> typename DEAFBABE::Material {
    typename DEAFBABE::Material dbMat = {};
    dbMat.setUnknown(mat.unknown);
    dbMat.setSurfaceStone(mat.surfaceStone);
    dbMat.setSurfaceMetal(mat.surfaceMetal);
    dbMat.setSurfaceGrass(mat.surfaceGrass);
    dbMat.setSurfaceIce(mat.surfaceIce);
    dbMat.setPillar(mat.pillar);
    dbMat.setSurfaceMetalGrating(mat.surfaceMetalGrating);
    dbMat.setSurfacePhazon(mat.surfacePhazon);
    dbMat.setSurfaceDirt(mat.surfaceDirt);
    dbMat.setSurfaceLava(mat.surfaceLava);
    dbMat.setSurfaceSPMetal(mat.surfaceSPMetal);
    dbMat.setSurfaceLavaStone(mat.surfaceLavaStone);
    dbMat.setSurfaceSnow(mat.surfaceSnow);
    dbMat.setSurfaceMudSlow(mat.surfaceMudSlow);
    dbMat.setSurfaceFabric(mat.surfaceFabric);
    dbMat.setHalfPipe(mat.halfPipe);
    dbMat.setSurfaceMud(mat.surfaceMud);
    dbMat.setSurfaceGlass(mat.surfaceGlass);
    dbMat.setUnused3(mat.unused3);
    dbMat.setUnused4(mat.unused4);
    dbMat.setSurfaceShield(mat.surfaceShield);
    dbMat.setSurfaceSand(mat.surfaceSand);
    dbMat.setSurfaceMothOrSeedOrganics(mat.surfaceMothOrSeedOrganics);
    dbMat.setSurfaceWeb(mat.surfaceWeb);
    dbMat.setProjectilePassthrough(mat.projPassthrough);
    dbMat.setSolid(mat.solid);
    dbMat.setNoPlatformCollision(mat.noPlatformCollision);
    dbMat.setCameraPassthrough(mat.camPassthrough);
    dbMat.setSurfaceWood(mat.surfaceWood);
    dbMat.setSurfaceOrganic(mat.surfaceOrganic);
    dbMat.setNoEdgeCollision(mat.noEdgeCollision);
    dbMat.setSurfaceRubber(mat.surfaceRubber);
    dbMat.setSeeThrough(mat.seeThrough);
    dbMat.setScanPassthrough(mat.scanPassthrough);
    dbMat.setAiPassthrough(mat.aiPassthrough);
    dbMat.setCeiling(mat.ceiling);
    dbMat.setWall(mat.wall);
    dbMat.setFloor(mat.floor);
    dbMat.setAiBlock(mat.aiBlock);
    dbMat.setJumpNotAllowed(mat.jumpNotAllowed);
    dbMat.setSpiderBall(mat.spiderBall);
    dbMat.setScrewAttackWallJump(mat.screwAttackWallJump);
    dbMat.setFlipFace(flipFace);
    return dbMat;
  };

  MaterialPool matPool;
  db.materials.reserve(colMesh.materials.size() * 2);

  zeus::CAABox fullAABB;

  db.verts.reserve(colMesh.verts.size());
  db.vertMats.resize(colMesh.verts.size());
  for (const auto& vert : colMesh.verts) {
    fullAABB.accumulateBounds(zeus::CVector3f(vert));
    db.verts.push_back(vert);
  }
  db.vertMatsCount = colMesh.verts.size();
  db.vertCount = colMesh.verts.size();

  db.edgeVertConnections.reserve(colMesh.edges.size());
  db.edgeMats.resize(colMesh.edges.size());
  for (const auto& edge : colMesh.edges) {
    db.edgeVertConnections.emplace_back();
    db.edgeVertConnections.back().verts[0] = edge.verts[0];
    db.edgeVertConnections.back().verts[1] = edge.verts[1];
  }
  db.edgeMatsCount = colMesh.edges.size();
  db.edgeVertsCount = colMesh.edges.size();

  db.triMats.reserve(colMesh.trianges.size());
  db.triangleEdgeConnections.reserve(colMesh.trianges.size());
  for (const auto& tri : colMesh.trianges) {
    int triMatIdx = matPool.AddOrLookup(MakeMat(colMesh.materials[tri.matIdx], tri.flip), db.materials);
    db.triMats.push_back(triMatIdx);

    db.triangleEdgeConnections.emplace_back();
    db.triangleEdgeConnections.back().edges[0] = tri.edges[0];
    db.triangleEdgeConnections.back().edges[1] = tri.edges[1];
    db.triangleEdgeConnections.back().edges[2] = tri.edges[2];

    for (int e = 0; e < 3; ++e) {
      db.edgeMats[tri.edges[e]] = triMatIdx;
      for (int v = 0; v < 2; ++v)
        db.vertMats[colMesh.edges[e].verts[v]] = triMatIdx;
    }
  }
  db.triMatsCount = colMesh.trianges.size();
  db.triangleEdgesCount = colMesh.trianges.size() * 3;

  db.materialCount = db.materials.size();

  PopulateAreaFields(db, colMesh, fullAABB);
}

template void DeafBabeBuildFromBlender<DNAMP1::DeafBabe>(DNAMP1::DeafBabe& db, const hecl::blender::ColMesh& colMesh);
template void DeafBabeBuildFromBlender<DNAMP2::DeafBabe>(DNAMP2::DeafBabe& db, const hecl::blender::ColMesh& colMesh);
template void DeafBabeBuildFromBlender<DNAMP1::DCLN::Collision>(DNAMP1::DCLN::Collision& db,
                                                                const hecl::blender::ColMesh& colMesh);

} // namespace DataSpec
