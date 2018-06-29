#include "PATH.hpp"
#include "hecl/Blender/Connection.hpp"
#include "zeus/CAABox.hpp"
#include "DataSpec/DNACommon/AROTBuilder.hpp"

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
              "bpy.types.Material.retro_path_idx_mask = bpy.props.IntProperty(name='Retro: Path Index Mask')\n"
              "bpy.types.Material.retro_path_type_mask = bpy.props.IntProperty(name='Retro: Path Type Mask')\n"
              "\n"
              "material_dict = {}\n"
              "material_index = []\n"
              "def make_ground_material(idxMask):\n"
              "    mat = bpy.data.materials.new('Ground %%X' %% idxMask)\n"
              "    mat.diffuse_color = (0.8, 0.460, 0.194)\n"
              "    return mat\n"
              "def make_flyer_material(idxMask):\n"
              "    mat = bpy.data.materials.new('Flyer %%X' %% idxMask)\n"
              "    mat.diffuse_color = (0.016, 0.8, 0.8)\n"
              "    return mat\n"
              "def make_swimmer_material(idxMask):\n"
              "    mat = bpy.data.materials.new('Swimmer %%X' %% idxMask)\n"
              "    mat.diffuse_color = (0.074, 0.293, 0.8)\n"
              "    return mat\n"
              "def select_material(meshIdxMask, meshTypeMask):\n"
              "    key = (meshIdxMask, meshTypeMask)\n"
              "    if key in material_index:\n"
              "        return material_index.index(key)\n"
              "    elif key in material_dict:\n"
              "        material_index.append(key)\n"
              "        return len(material_index)-1\n"
              "    else:\n"
              "        if meshTypeMask == 0x2:\n"
              "            mat = make_flyer_material(meshIdxMask)\n"
              "        elif meshTypeMask == 0x4:\n"
              "            mat = make_swimmer_material(meshIdxMask)\n"
              "        else:\n"
              "            mat = make_ground_material(meshIdxMask)\n"
              "        mat.retro_path_idx_mask = meshIdxMask\n"
              "        mat.retro_path_type_mask = meshTypeMask\n"
              "        material_dict[key] = mat\n"
              "        material_index.append(key)\n"
              "        return len(material_index)-1\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    if ob.type != 'CAMERA':\n"
              "        bpy.context.scene.objects.unlink(ob)\n"
              "        bpy.data.objects.remove(ob)\n"
              "\n"
              "bm = bmesh.new()\n"
              "height_lay = bm.faces.layers.float.new('Height')\n",
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

        os.format("face = bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = bm.faces.new(tri_verts)\n"
                  "    face.normal_flip()\n"
                  "face.material_index = select_material(0x%04X, 0x%04X)\n"
                  "face.smooth = False\n"
                  "face[height_lay] = %f\n"
                  "\n", r.meshIndexMask, r.meshTypeMask, r.height);

#if 0
        zeus::CVector3f center = xf->multiplyOneOverW(r.centroid);
        zeus::CAABox aabb(xf->multiplyOneOverW(r.aabb[0]), xf->multiplyOneOverW(r.aabb[1]));
        os.format("aabb = bpy.data.objects.new('AABB', None)\n"
                  "aabb.location = (%f,%f,%f)\n"
                  "aabb.scale = (%f,%f,%f)\n"
                  "aabb.empty_draw_type = 'CUBE'\n"
                  "bpy.context.scene.objects.link(aabb)\n"
                  "centr = bpy.data.objects.new('Center', None)\n"
                  "centr.location = (%f,%f,%f)\n"
                  "bpy.context.scene.objects.link(centr)\n",
                  aabb.min.v[0] + (aabb.max.v[0] - aabb.min.v[0]) / 2.f,
                  aabb.min.v[1] + (aabb.max.v[1] - aabb.min.v[1]) / 2.f,
                  aabb.min.v[2] + (aabb.max.v[2] - aabb.min.v[2]) / 2.f,
                  (aabb.max.v[0] - aabb.min.v[0]) / 2.f,
                  (aabb.max.v[1] - aabb.min.v[1]) / 2.f,
                  (aabb.max.v[2] - aabb.min.v[2]) / 2.f,
                  center.x, center.y, center.z);
#endif
    }

    os << "bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=0.001)\n"
          "path_mesh = bpy.data.meshes.new('PATH')\n"
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
                const PathMesh& mesh)
{
    athena::io::MemoryReader r(mesh.data.data(), mesh.data.size());
    PATH path;
    path.read(r);
    if (!path.regions.empty())
    {
        AROTBuilder octreeBuilder;
        octreeBuilder.buildPath(path);
    }
    else
    {
        path.octreeNodeCount = 1;
        path.octree.emplace_back();
        OctreeNode& n = path.octree.back();
        n.isLeaf = 1;
        n.aabb[0] = zeus::CVector3f{FLT_MAX, FLT_MAX, FLT_MAX};
        n.aabb[1] = zeus::CVector3f{-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (int i=0 ; i<8 ; ++i)
            n.children[i] = 0xffffffff;
    }

    athena::io::FileWriter w(outPath.getAbsolutePath());
    path.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}

}