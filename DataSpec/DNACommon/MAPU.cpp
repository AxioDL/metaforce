#include "MAPU.hpp"
#include "../DNAMP1/DNAMP1.hpp"
#include "../DNAMP2/DNAMP2.hpp"
#include "../DNAMP3/DNAMP3.hpp"
#include "zeus/CTransform.hpp"

namespace DataSpec
{
namespace DNAMAPU
{

template <typename PAKRouter>
bool ReadMAPUToBlender(hecl::BlenderConnection& conn,
                       const MAPU& mapu,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force)
{
    if (!force && outPath.isFile())
        return true;

    if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::MapUniverse))
        return false;
    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os << "import bpy\n"
          "from mathutils import Matrix\n"
          "\n"
          "# Clear Scene\n"
          "bpy.context.scene.camera = None\n"
          "for ob in bpy.data.objects:\n"
          "    bpy.context.scene.objects.unlink(ob)\n"
          "    bpy.data.objects.remove(ob)\n"
          "\n"
          "bpy.types.Object.retro_mapworld_color = bpy.props.FloatVectorProperty(name='Retro: MapWorld Color',"
          " description='Sets map world color', subtype='COLOR', size=4, min=0.0, max=1.0)\n"
          "bpy.types.Object.retro_mapworld_path = bpy.props.StringProperty(name='Retro: MapWorld Path',"
          " description='Sets path to World root')\n"
          "\n";

    hecl::ProjectPath hexPath = pakRouter.getWorking(mapu.hexMapa);
    os.linkBlend(hexPath.getAbsolutePathUTF8().c_str(),
                 pakRouter.getBestEntryName(mapu.hexMapa).c_str());
    os << "hexMesh = bpy.data.objects['MAP'].data\n";

    for (const MAPU::World& wld : mapu.worlds)
    {
        hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(wld.mlvl);
        const MAPU::Transform& wldXf = wld.transform;
        os.format("wldObj = bpy.data.objects.new('%s', None)\n"
                  "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "wldObj.rotation_mode = 'QUATERNION'\n"
                  "wldObj.location = mtxd[0]\n"
                  "wldObj.rotation_quaternion = mtxd[1]\n"
                  "wldObj.scale = mtxd[2]\n"
                  "wldObj.retro_mapworld_color = (%f, %f, %f, %f)\n"
                  "wldObj.retro_mapworld_path = '''%s'''\n"
                  "bpy.context.scene.objects.link(wldObj)\n", wld.name.c_str(),
                  wldXf.xf[0].vec[0], wldXf.xf[0].vec[1], wldXf.xf[0].vec[2], wldXf.xf[0].vec[3],
                  wldXf.xf[1].vec[0], wldXf.xf[1].vec[1], wldXf.xf[1].vec[2], wldXf.xf[1].vec[3],
                  wldXf.xf[2].vec[0], wldXf.xf[2].vec[1], wldXf.xf[2].vec[2], wldXf.xf[2].vec[3],
                  wld.hexColor.r, wld.hexColor.g, wld.hexColor.b, wld.hexColor.a,
                  path.getParentPath().getRelativePathUTF8().c_str());
        int idx = 0;
        for (const MAPU::Transform& hexXf : wld.hexTransforms)
        {
            os.format("obj = bpy.data.objects.new('%s_%d', hexMesh)\n"
                      "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                      "mtxd = mtx.decompose()\n"
                      "obj.rotation_mode = 'QUATERNION'\n"
                      "obj.location = mtxd[0]\n"
                      "obj.rotation_quaternion = mtxd[1]\n"
                      "obj.scale = mtxd[2]\n"
                      "bpy.context.scene.objects.link(obj)\n"
                      "obj.parent = wldObj\n",
                      wld.name.c_str(), idx++,
                      hexXf.xf[0].vec[0], hexXf.xf[0].vec[1], hexXf.xf[0].vec[2], hexXf.xf[0].vec[3],
                      hexXf.xf[1].vec[0], hexXf.xf[1].vec[1], hexXf.xf[1].vec[2], hexXf.xf[1].vec[3],
                      hexXf.xf[2].vec[0], hexXf.xf[2].vec[1], hexXf.xf[2].vec[2], hexXf.xf[2].vec[3]);
        }
    }

    os << "for screen in bpy.data.screens:\n"
          "    for area in screen.areas:\n"
          "        for space in area.spaces:\n"
          "            if space.type == 'VIEW_3D':\n"
          "                space.viewport_shade = 'SOLID'\n"
          "                space.clip_end = 8000.0\n";

    os.centerView();
    os.close();
    return conn.saveBlend();
}

template bool ReadMAPUToBlender<PAKRouter<DNAMP1::PAKBridge>>
(hecl::BlenderConnection& conn,
 const MAPU& mapu,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP1::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
 bool force);

template bool ReadMAPUToBlender<PAKRouter<DNAMP2::PAKBridge>>
(hecl::BlenderConnection& conn,
 const MAPU& mapu,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP2::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
 bool force);

bool MAPU::Cook(const hecl::BlenderConnection::DataStream::MapUniverse& mapuIn, const hecl::ProjectPath& out)
{
    MAPU mapu;

    mapu.magic = 0xABCDEF01;
    mapu.version = 1;
    mapu.hexMapa = mapuIn.hexagonPath;

    mapu.worldCount = mapuIn.worlds.size();
    mapu.worlds.reserve(mapuIn.worlds.size());
    for (const hecl::BlenderConnection::DataStream::MapUniverse::World& wld : mapuIn.worlds)
    {
        mapu.worlds.emplace_back();
        MAPU::World& wldOut = mapu.worlds.back();
        wldOut.name = wld.name;
        wldOut.mlvl = hecl::ProjectPath(wld.worldPath, _S("!world.*"));
        wldOut.transform.xf[0] = wld.xf.val[0];
        wldOut.transform.xf[1] = wld.xf.val[1];
        wldOut.transform.xf[2] = wld.xf.val[2];
        wldOut.hexCount = wld.hexagons.size();
        wldOut.hexTransforms.reserve(wld.hexagons.size());
        for (const hecl::BlenderConnection::DataStream::Matrix4f& mtx : wld.hexagons)
        {
            wldOut.hexTransforms.emplace_back();
            MAPU::Transform& xf = wldOut.hexTransforms.back();
            xf.xf[0] = mtx.val[0];
            xf.xf[1] = mtx.val[1];
            xf.xf[2] = mtx.val[2];
        }
        wldOut.hexColor = zeus::CColor(wld.color.val);
    }

    athena::io::FileWriter f(out.getAbsolutePath());
    mapu.write(f);
    return true;
}

}
}
