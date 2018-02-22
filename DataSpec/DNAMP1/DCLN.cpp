#include "DCLN.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1
{

#if DCLN_DUMP_OBB
void DCLN::Collision::NodesendToBlender(hecl::blender::PyOutStream& os) const
{
    os.format("obj = bpy.data.objects.new('%s', None)\n"
              "obj.empty_draw_type = 'CUBE'\n"
              "bpy.context.scene.objects.link(obj)\n"
              "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
              "mtxd = mtx.decompose()\n"
              "obj.rotation_mode = 'QUATERNION'\n"
              "obj.location = mtxd[0]\n"
              "obj.rotation_quaternion = mtxd[1]\n"
              "obj.scale = (%f,%f,%f)\n", isLeaf ? "leaf" : "branch",
              xf[0].vec[0], xf[0].vec[1], xf[0].vec[2], xf[0].vec[3],
              xf[1].vec[0], xf[1].vec[1], xf[1].vec[2], xf[1].vec[3],
              xf[2].vec[0], xf[2].vec[1], xf[2].vec[2], xf[2].vec[3],
              halfExtent.vec[0], halfExtent.vec[1], halfExtent.vec[2]);
    if (isLeaf)
        os << "obj.show_name = True\n";
    if (!isLeaf)
    {
        left->sendToBlender(os);
        right->sendToBlender(os);
    }
}
#endif

template <class Op>
void DCLN::Collision::Node::Enumerate(typename Op::StreamT& s)
{
    Do<Op>({"xf[0]"}, xf[0], s);
    Do<Op>({"xf[1]"}, xf[1], s);
    Do<Op>({"xf[2]"}, xf[2], s);
    Do<Op>({"halfExtent"}, halfExtent, s);
    Do<Op>({"isLeaf"}, isLeaf, s);
    if (isLeaf)
    {
        if (!leafData)
            leafData.reset(new LeafData);
        Do<Op>({"leafData"}, *leafData, s);
    }
    else
    {
        if (!left)
            left.reset(new Node);
        Do<Op>({"left"}, *left, s);
        if (!right)
            right.reset(new Node);
        Do<Op>({"right"}, *right, s);
    }
}

AT_SPECIALIZE_DNA(DCLN::Collision::Node)

void DCLN::sendToBlender(hecl::blender::Connection& conn, std::string_view entryName)
{
    /* Open Py Stream and read sections */
    hecl::blender::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
                  "import bmesh\n"
                  "from mathutils import Vector, Matrix\n"
                  "\n"
                  "bpy.context.scene.name = '%s'\n"
                  "# Clear Scene\n"
                  "for ob in bpy.data.objects:\n"
                  "    if ob.type != 'CAMERA':\n"
                  "        bpy.context.scene.objects.unlink(ob)\n"
                  "        bpy.data.objects.remove(ob)\n",
              entryName.data());

    DeafBabe::BlenderInit(os);
    atInt32 idx = 0;
    for (const Collision& col : collision)
    {
        DeafBabeSendToBlender(os, col, true, idx++);
#if DCLN_DUMP_OBB
        col.root.sendToBlender(os);
#endif
    }
    os.centerView();
    os.close();
}

bool DCLN::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged)
{
    DCLN dcln;
    dcln.read(rs);
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.createBlend(outPath, hecl::blender::BlendType::ColMesh))
        return false;

    dcln.sendToBlender(conn, pakRouter.getBestEntryName(entry, false));
    return conn.saveBlend();
}

bool DCLN::Cook(const hecl::ProjectPath& outPath,
                const hecl::ProjectPath& inPath,
                const std::vector<Mesh>& meshes,
                hecl::blender::Connection* conn)
{
    DCLN dcln;
    dcln.colCount = atUint32(meshes.size());
    for (const Mesh& mesh : meshes)
    {
        dcln.collision.emplace_back();
        Collision& colOut = dcln.collision.back();
        DeafBabeBuildFromBlender(colOut, mesh);
        colOut.root = std::move(*OBBTreeBuilder::buildCol<Collision::Node>(mesh));
        colOut.memSize = atUint32(colOut.root.getMemoryUsage());
    }

    athena::io::FileWriter w(outPath.getAbsolutePath());
    dcln.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}

}
