#include "PATH.hpp"
#include "hecl/Blender/Connection.hpp"
#include <unordered_set>

namespace DataSpec::DNAMP1
{

void PATH::sendToBlender(hecl::blender::Connection& conn, std::string_view entryName,
                         const zeus::CMatrix4f* xf)
{
    /* Open Py Stream and read sections */
    hecl::blender::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Vector, Matrix\n"
              "\n"
              "material_dict = {}\n"
              "material_index = []\n"
              "def select_material(data):\n"
              "    if data in material_index:\n"
              "        return material_index.index(data)\n"
              "    elif data in material_dict:\n"
              "        material_index.append(data)\n"
              "        return len(material_index)-1\n"
              "    else:\n"
              "        mat = bpy.data.materials.new(data)\n"
              "        material_dict[data] = mat\n"
              "        material_index.append(data)\n"
              "        return len(material_index)-1\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    if ob.type != 'CAMERA':\n"
              "        bpy.context.scene.objects.unlink(ob)\n"
              "        bpy.data.objects.remove(ob)\n"
              "\n"
              "bm = bmesh.new()\n",
              entryName.data());

    for (const Node& n : nodes)
        os.format("bm.verts.new((%f,%f,%f))\n",
                  n.position.vec[0], n.position.vec[1], n.position.vec[2]);

    os << "bm.verts.ensure_lookup_table()\n";

    for (const Region& r : regions)
    {
        os << "tri_verts = []\n";
        for (int i=0 ; i<r.nodeCount ; ++i)
            os.format("tri_verts.append(bm.verts[%u])\n", r.nodeStart + i);

        zeus::CVector3f centroid = r.centroid;
        if (xf)
            centroid = xf->multiplyOneOverW(centroid);
        os.format("face = bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = bm.faces.new(tri_verts)\n"
                  "    face.normal_flip()\n"
                  "face.material_index = select_material('0x%08X')\n"
                  "face.smooth = False\n"
                  "hobj = bpy.data.objects.new('Height', None)\n"
                  "hobj.location = (%f,%f,%f)\n"
                  "hobj.layers[1] = True\n"
                  "bpy.context.scene.objects.link(hobj)\n"
                  "\n", r.flags, centroid.v[0], centroid.v[1], centroid.v[2] + r.height);
    }

    os << "path_mesh = bpy.data.meshes.new('PATH')\n"
          "bm.to_mesh(path_mesh)\n"
          "path_mesh_obj = bpy.data.objects.new(path_mesh.name, path_mesh)\n"
          "\n"
          "for mat_name in material_index:\n"
          "    mat = material_dict[mat_name]\n"
          "    path_mesh.materials.append(mat)\n"
          "\n"
          "bpy.context.scene.objects.link(path_mesh_obj)\n"
          "path_mesh_obj.draw_type = 'SOLID'\n"
          "path_mesh_obj.game.physics_type = 'STATIC'\n"
          "path_mesh_obj.layers[1] = True\n"
          "bpy.context.scene.hecl_path_obj = path_mesh_obj.name\n"
          "\n"
          "for ar in bpy.context.screen.areas:\n"
          "    for sp in ar.spaces:\n"
          "        if sp.type == 'VIEW_3D':\n"
          "            sp.viewport_shade = 'SOLID'\n";

    if (xf)
    {
        const zeus::CMatrix4f& w = *xf;
        os.format("mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "path_mesh_obj.rotation_mode = 'QUATERNION'\n"
                  "path_mesh_obj.location = mtxd[0]\n"
                  "path_mesh_obj.rotation_quaternion = mtxd[1]\n"
                  "path_mesh_obj.scale = mtxd[2]\n",
                  w.m[0][0], w.m[1][0], w.m[2][0], w.m[3][0],
                  w.m[0][1], w.m[1][1], w.m[2][1], w.m[3][1],
                  w.m[0][2], w.m[1][2], w.m[2][2], w.m[3][2]);
    }

    os.linkBackground("//!area.blend");
    os.centerView();
    os.close();
}

bool PATH::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged)
{
    PATH path;
    path.read(rs);
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.createBlend(outPath, hecl::blender::BlendType::PathMesh))
        return false;

    const zeus::CMatrix4f* xf = pakRouter.lookupMAPATransform(entry.id);
    path.sendToBlender(conn, pakRouter.getBestEntryName(entry, false), xf);
    return conn.saveBlend();
}

bool PATH::Cook(const hecl::ProjectPath& outPath,
                const hecl::ProjectPath& inPath,
                const PathMesh& mesh,
                hecl::blender::Connection* conn)
{
    PATH path;

    athena::io::FileWriter w(outPath.getAbsolutePath());
    path.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}

}