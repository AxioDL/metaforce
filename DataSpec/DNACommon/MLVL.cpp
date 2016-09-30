#include "MLVL.hpp"
#include "../DNAMP1/MLVL.hpp"
#include "../DNAMP2/MLVL.hpp"
#include "../DNAMP3/MLVL.hpp"

namespace DataSpec
{
namespace DNAMLVL
{

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(hecl::BlenderConnection& conn,
                       const MLVL& mlvl,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force,
                       std::function<void(const hecl::SystemChar*)> fileChanged)
{
    if (!force && outPath.isFile())
        return true;

    /* Create World Blend */
    if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::World))
        return false;
    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Matrix\n"
              "\n"
              "bpy.context.scene.name = 'World'\n"
              "\n"
              "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    if ob.type != 'CAMERA':\n"
              "        bpy.context.scene.objects.unlink(ob)\n"
              "        bpy.data.objects.remove(ob)\n");

    /* Insert area empties */
    int areaIdx = 0;
    for (const auto& area : mlvl.areas)
    {
        const typename PAKRouter::EntryType* mreaEntry = pakRouter.lookupEntry(area.areaMREAId);
        hecl::SystemUTF8View areaDirName(*mreaEntry->unique.m_areaName);

        os.AABBToBMesh(area.aabb[0], area.aabb[1]);
        os.format("box_mesh = bpy.data.meshes.new('''%s''')\n"
                  "bm.to_mesh(box_mesh)\n"
                  "bm.free()\n"
                  "box = bpy.data.objects.new(box_mesh.name, box_mesh)\n"
                  "bpy.context.scene.objects.link(box)\n"
                  "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "box.rotation_mode = 'QUATERNION'\n"
                  "box.location = mtxd[0]\n"
                  "box.rotation_quaternion = mtxd[1]\n"
                  "box.scale = mtxd[2]\n",
                  areaDirName.str().c_str(),
                  area.transformMtx[0].vec[0], area.transformMtx[0].vec[1], area.transformMtx[0].vec[2], area.transformMtx[0].vec[3],
                  area.transformMtx[1].vec[0], area.transformMtx[1].vec[1], area.transformMtx[1].vec[2], area.transformMtx[1].vec[3],
                  area.transformMtx[2].vec[0], area.transformMtx[2].vec[1], area.transformMtx[2].vec[2], area.transformMtx[2].vec[3]);

        /* Insert dock planes */
        int dockIdx = 0;
        for (const auto& dock : area.docks)
        {
            os << "bm = bmesh.new()\n";
            zeus::CVector3f pvAvg;
            for (const atVec3f& pv : dock.planeVerts)
                pvAvg += pv;
            pvAvg /= dock.planeVerts.size();
            int idx = 0;
            for (const atVec3f& pv : dock.planeVerts)
            {
                zeus::CVector3f pvRel = zeus::CVector3f(pv) - pvAvg;
                os.format("bm.verts.new((%f,%f,%f))\n"
                          "bm.verts.ensure_lookup_table()\n",
                          pvRel[0], pvRel[1], pvRel[2]);
                if (idx)
                    os << "bm.edges.new((bm.verts[-2], bm.verts[-1]))\n";
                ++idx;
            }
            os << "bm.edges.new((bm.verts[-1], bm.verts[0]))\n";
            os.format("dockMesh = bpy.data.meshes.new('DOCK_%02d_%02d')\n", areaIdx, dockIdx);
            os << "dockObj = bpy.data.objects.new(dockMesh.name, dockMesh)\n"
                  "bpy.context.scene.objects.link(dockObj)\n"
                  "bm.to_mesh(dockMesh)\n"
                  "bm.free()\n"
                  "dockObj.parent = box\n";
            os.format("dockObj.location = (%f,%f,%f)\n",
                      pvAvg[0], pvAvg[1], pvAvg[2]);
            ++dockIdx;
        }
        ++areaIdx;
    }

    os.centerView();
    os.close();
    conn.saveBlend();
    return true;
}

template bool ReadMLVLToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MLVL>
(hecl::BlenderConnection& conn,
 const DNAMP1::MLVL& mlvl,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP1::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
 bool force,
 std::function<void(const hecl::SystemChar*)> fileChanged);

template bool ReadMLVLToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::MLVL>
(hecl::BlenderConnection& conn,
 const DNAMP2::MLVL& mlvl,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP2::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
 bool force,
 std::function<void(const hecl::SystemChar*)> fileChanged);

template bool ReadMLVLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MLVL>
(hecl::BlenderConnection& conn,
 const DNAMP3::MLVL& mlvl,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP3::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
 bool force,
 std::function<void(const hecl::SystemChar*)> fileChanged);

}
}
