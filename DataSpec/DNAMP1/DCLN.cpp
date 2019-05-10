#include "DCLN.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

#if DCLN_DUMP_OBB
void DCLN::Collision::Node::sendToBlender(hecl::blender::PyOutStream& os) const {
  os.format(
      "obj = bpy.data.objects.new('%s', None)\n"
      "obj.empty_display_type = 'CUBE'\n"
      "bpy.context.scene.collection.objects.link(obj)\n"
      "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
      "mtxd = mtx.decompose()\n"
      "obj.rotation_mode = 'QUATERNION'\n"
      "obj.location = mtxd[0]\n"
      "obj.rotation_quaternion = mtxd[1]\n"
      "obj.scale = (%f,%f,%f)\n",
      isLeaf ? "leaf" : "branch", xf[0].simd[0], xf[0].simd[1], xf[0].simd[2], xf[0].simd[3], xf[1].simd[0],
      xf[1].simd[1], xf[1].simd[2], xf[1].simd[3], xf[2].simd[0], xf[2].simd[1], xf[2].simd[2], xf[2].simd[3],
      halfExtent.simd[0], halfExtent.simd[1], halfExtent.simd[2]);
  if (isLeaf)
    os << "obj.show_name = True\n";
  if (!isLeaf) {
    left->sendToBlender(os);
    right->sendToBlender(os);
  }
}
#endif

void DCLN::sendToBlender(hecl::blender::Connection& conn, std::string_view entryName) {
  /* Open Py Stream and read sections */
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);
  os.format(
      "import bpy\n"
      "import bmesh\n"
      "from mathutils import Vector, Matrix\n"
      "\n"
      "bpy.context.scene.name = '%s'\n"
      "# Clear Scene\n"
      "if 'Collection 1' in bpy.data.collections:\n"
      "    bpy.data.collections.remove(bpy.data.collections['Collection 1'])\n",
      entryName.data());

  DeafBabe::BlenderInit(os);
  atInt32 idx = 0;
  for (const Collision& col : collision) {
    DeafBabeSendToBlender(os, col, true, idx++);
#if DCLN_DUMP_OBB
    col.root.sendToBlender(os);
#endif
  }
  os.centerView();
  os.close();
}

bool DCLN::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged) {
  DCLN dcln;
  dcln.read(rs);
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::ColMesh))
    return false;

  dcln.sendToBlender(conn, pakRouter.getBestEntryName(entry, false));
  return conn.saveBlend();
}

bool DCLN::Cook(const hecl::ProjectPath& outPath, const std::vector<Mesh>& meshes) {
  DCLN dcln;
  dcln.colCount = atUint32(meshes.size());
  for (const Mesh& mesh : meshes) {
    dcln.collision.emplace_back();
    Collision& colOut = dcln.collision.back();
    DeafBabeBuildFromBlender(colOut, mesh);
    colOut.root = std::move(*OBBTreeBuilder::buildCol<Collision::Node>(mesh));
    colOut.memSize = atUint32(colOut.root.getMemoryUsage());
  }

#if DCLN_DUMP_OBB
  hecl::blender::Connection& conn = hecl::blender::SharedBlenderToken.getBlenderConnection();
  conn.createBlend(outPath.getWithExtension(_SYS_STR(".blend")), hecl::blender::BlendType::ColMesh);
  dcln.sendToBlender(conn, "BLAH");
  conn.saveBlend();
#endif

  athena::io::FileWriter w(outPath.getAbsolutePath());
  dcln.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}

} // namespace DataSpec::DNAMP1
