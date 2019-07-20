#include "MAPU.hpp"
#include "../DNAMP1/DNAMP1.hpp"
#include "../DNAMP2/DNAMP2.hpp"
#include "../DNAMP3/DNAMP3.hpp"
#include "zeus/CTransform.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMAPU {

template <typename PAKRouter>
bool ReadMAPUToBlender(hecl::blender::Connection& conn, const MAPU& mapu, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, bool force) {
  if (!force && outPath.isFile())
    return true;

  if (!conn.createBlend(outPath, hecl::blender::BlendType::MapUniverse))
    return false;
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);

  os << "import bpy\n"
        "from mathutils import Matrix\n"
        "\n"
        "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "bpy.types.Object.retro_mapworld_color = bpy.props.FloatVectorProperty(name='Retro: MapWorld Color',"
        " description='Sets map world color', subtype='COLOR', size=4, min=0.0, max=1.0)\n"
        "bpy.types.Object.retro_mapworld_path = bpy.props.StringProperty(name='Retro: MapWorld Path',"
        " description='Sets path to World root')\n"
        "\n";

  hecl::ProjectPath hexPath = pakRouter.getWorking(mapu.hexMapa);
  os.linkBlend(hexPath.getAbsolutePathUTF8().data(), pakRouter.getBestEntryName(mapu.hexMapa).data());
  os << "hexMesh = bpy.data.objects['MAP'].data\n";

  for (const MAPU::World& wld : mapu.worlds) {
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(wld.mlvl);
    const MAPU::Transform& wldXf = wld.transform;
    zeus::simd_floats wldXfF[3];
    for (int i = 0; i < 3; ++i)
      wldXf.xf[i].simd.copy_to(wldXfF[i]);
    zeus::simd_floats hexColorF(wld.hexColor.mSimd);
    os.format(fmt(
        "wldObj = bpy.data.objects.new('{}', None)\n"
        "mtx = Matrix((({},{},{},{}),({},{},{},{}),({},{},{},{}),(0.0,0.0,0.0,1.0)))\n"
        "mtxd = mtx.decompose()\n"
        "wldObj.rotation_mode = 'QUATERNION'\n"
        "wldObj.location = mtxd[0]\n"
        "wldObj.rotation_quaternion = mtxd[1]\n"
        "wldObj.scale = mtxd[2]\n"
        "wldObj.retro_mapworld_color = ({}, {}, {}, {})\n"
        "wldObj.retro_mapworld_path = '''{}'''\n"
        "bpy.context.scene.collection.objects.link(wldObj)\n"),
        wld.name, wldXfF[0][0], wldXfF[0][1], wldXfF[0][2], wldXfF[0][3], wldXfF[1][0], wldXfF[1][1],
        wldXfF[1][2], wldXfF[1][3], wldXfF[2][0], wldXfF[2][1], wldXfF[2][2], wldXfF[2][3], hexColorF[0], hexColorF[1],
        hexColorF[2], hexColorF[3], path.getParentPath().getRelativePathUTF8());
    int idx = 0;
    for (const MAPU::Transform& hexXf : wld.hexTransforms) {
      zeus::simd_floats hexXfF[3];
      for (int i = 0; i < 3; ++i)
        hexXf.xf[i].simd.copy_to(hexXfF[i]);
      os.format(fmt(
          "obj = bpy.data.objects.new('{}_{}', hexMesh)\n"
          "mtx = Matrix((({},{},{},{}),({},{},{},{}),({},{},{},{}),(0.0,0.0,0.0,1.0)))\n"
          "mtxd = mtx.decompose()\n"
          "obj.rotation_mode = 'QUATERNION'\n"
          "obj.location = mtxd[0]\n"
          "obj.rotation_quaternion = mtxd[1]\n"
          "obj.scale = mtxd[2]\n"
          "bpy.context.scene.collection.objects.link(obj)\n"
          "obj.parent = wldObj\n"),
          wld.name, idx++, hexXfF[0][0], hexXfF[0][1], hexXfF[0][2], hexXfF[0][3], hexXfF[1][0], hexXfF[1][1],
          hexXfF[1][2], hexXfF[1][3], hexXfF[2][0], hexXfF[2][1], hexXfF[2][2], hexXfF[2][3]);
    }
  }

  os << "for screen in bpy.data.screens:\n"
        "    for area in screen.areas:\n"
        "        for space in area.spaces:\n"
        "            if space.type == 'VIEW_3D':\n"
        "                space.clip_end = 8000.0\n";

  os.centerView();
  os.close();
  conn.saveBlend();
  return true;
}

template bool ReadMAPUToBlender<PAKRouter<DNAMP1::PAKBridge>>(hecl::blender::Connection& conn, const MAPU& mapu,
                                                              const hecl::ProjectPath& outPath,
                                                              PAKRouter<DNAMP1::PAKBridge>& pakRouter,
                                                              const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
                                                              bool force);

template bool ReadMAPUToBlender<PAKRouter<DNAMP2::PAKBridge>>(hecl::blender::Connection& conn, const MAPU& mapu,
                                                              const hecl::ProjectPath& outPath,
                                                              PAKRouter<DNAMP2::PAKBridge>& pakRouter,
                                                              const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
                                                              bool force);

bool MAPU::Cook(const hecl::blender::MapUniverse& mapuIn, const hecl::ProjectPath& out) {
  MAPU mapu;

  mapu.magic = 0xABCDEF01;
  mapu.version = 1;
  mapu.hexMapa = mapuIn.hexagonPath;

  mapu.worldCount = mapuIn.worlds.size();
  mapu.worlds.reserve(mapuIn.worlds.size());
  for (const hecl::blender::MapUniverse::World& wld : mapuIn.worlds) {
    mapu.worlds.emplace_back();
    MAPU::World& wldOut = mapu.worlds.back();
    wldOut.name = wld.name;
    wldOut.mlvl = hecl::ProjectPath(wld.worldPath, _SYS_STR("!world.*"));
    wldOut.transform.xf[0] = wld.xf.val[0];
    wldOut.transform.xf[1] = wld.xf.val[1];
    wldOut.transform.xf[2] = wld.xf.val[2];
    wldOut.hexCount = wld.hexagons.size();
    wldOut.hexTransforms.reserve(wld.hexagons.size());
    for (const hecl::blender::Matrix4f& mtx : wld.hexagons) {
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
  int64_t rem = f.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      f.writeBytes((atInt8*)"\xff", 1);
  return true;
}

} // namespace DataSpec::DNAMAPU
