#include "MLVL.hpp"
#include "../DNAMP1/MLVL.hpp"
#include "../DNAMP2/MLVL.hpp"
#include "../DNAMP3/MLVL.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMLVL {

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(hecl::blender::Connection& conn, const MLVL& mlvl, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, bool force,
                       std::function<void(const hecl::SystemChar*)> fileChanged) {
  hecl::ProjectPath blendPath = outPath.getWithExtension(_SYS_STR(".blend"), true);
  if (!force && blendPath.isFile())
    return true;

  /* Create World Blend */
  if (!conn.createBlend(blendPath, hecl::blender::BlendType::World))
    return false;
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);
  os << "import bpy\n"
        "import bmesh\n"
        "from mathutils import Matrix\n"
        "\n"
        "bpy.context.scene.name = 'World'\n"
        "\n"
        "# Clear Scene\n"
        "if 'Collection 1' in bpy.data.collections:\n"
        "    bpy.data.collections.remove(bpy.data.collections['Collection 1'])\n";

  /* Insert area empties */
  int areaIdx = 0;
  for (const auto& area : mlvl.areas) {
    const typename PAKRouter::EntryType* mreaEntry = pakRouter.lookupEntry(area.areaMREAId);
    hecl::SystemUTF8Conv areaDirName(*mreaEntry->unique.m_areaName);

    os.AABBToBMesh(area.aabb[0], area.aabb[1]);
    zeus::simd_floats xfMtxF[3];
    for (int i = 0; i < 3; ++i)
      area.transformMtx[i].simd.copy_to(xfMtxF[i]);
    os.format(
        "box_mesh = bpy.data.meshes.new('''%s''')\n"
        "bm.to_mesh(box_mesh)\n"
        "bm.free()\n"
        "box = bpy.data.objects.new(box_mesh.name, box_mesh)\n"
        "bpy.context.scene.collection.objects.link(box)\n"
        "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
        "mtxd = mtx.decompose()\n"
        "box.rotation_mode = 'QUATERNION'\n"
        "box.location = mtxd[0]\n"
        "box.rotation_quaternion = mtxd[1]\n"
        "box.scale = mtxd[2]\n",
        areaDirName.str().data(), xfMtxF[0][0], xfMtxF[0][1], xfMtxF[0][2], xfMtxF[0][3], xfMtxF[1][0], xfMtxF[1][1],
        xfMtxF[1][2], xfMtxF[1][3], xfMtxF[2][0], xfMtxF[2][1], xfMtxF[2][2], xfMtxF[2][3]);

    /* Insert dock planes */
    int dockIdx = 0;
    for (const auto& dock : area.docks) {
      os << "bm = bmesh.new()\n";
      zeus::CVector3f pvAvg;
      for (const atVec3f& pv : dock.planeVerts)
        pvAvg += pv;
      pvAvg /= zeus::CVector3f(dock.planeVerts.size());
      int idx = 0;
      for (const atVec3f& pv : dock.planeVerts) {
        const zeus::CVector3f pvRel = zeus::CVector3f(pv) - pvAvg;
        os.format(
            "bm.verts.new((%f,%f,%f))\n"
            "bm.verts.ensure_lookup_table()\n",
            pvRel[0], pvRel[1], pvRel[2]);
        if (idx)
          os << "bm.edges.new((bm.verts[-2], bm.verts[-1]))\n";
        ++idx;
      }
      os << "bm.edges.new((bm.verts[-1], bm.verts[0]))\n";
      os.format("dockMesh = bpy.data.meshes.new('DOCK_%02d_%02d')\n", areaIdx, dockIdx);
      os << "dockObj = bpy.data.objects.new(dockMesh.name, dockMesh)\n"
            "bpy.context.scene.collection.objects.link(dockObj)\n"
            "bm.to_mesh(dockMesh)\n"
            "bm.free()\n"
            "dockObj.parent = box\n";
      os.format("dockObj.location = (%f,%f,%f)\n", float(pvAvg[0]), float(pvAvg[1]), float(pvAvg[2]));
      ++dockIdx;
    }
    ++areaIdx;
  }

  os.centerView();
  os.close();
  conn.saveBlend();
  return true;
}

template bool ReadMLVLToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MLVL>(
    hecl::blender::Connection& conn, const DNAMP1::MLVL& mlvl, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP1::PAKBridge>& pakRouter, const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry, bool force,
    std::function<void(const hecl::SystemChar*)> fileChanged);

template bool ReadMLVLToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::MLVL>(
    hecl::blender::Connection& conn, const DNAMP2::MLVL& mlvl, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP2::PAKBridge>& pakRouter, const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry, bool force,
    std::function<void(const hecl::SystemChar*)> fileChanged);

template bool ReadMLVLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MLVL>(
    hecl::blender::Connection& conn, const DNAMP3::MLVL& mlvl, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP3::PAKBridge>& pakRouter, const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry, bool force,
    std::function<void(const hecl::SystemChar*)> fileChanged);

} // namespace DataSpec::DNAMLVL
