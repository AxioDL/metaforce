#include <Athena/FileWriter.hpp>
#include "MREA.hpp"
#include "../DNAMP2/DeafBabe.hpp"

namespace Retro
{
namespace DNAMP3
{

MREA::StreamReader::StreamReader(Athena::io::IStreamReader& source,
                                 atUint32 blkCount, atUint32 secIdxCount)
: DNAMP2::MREA::StreamReader(source)
{
    m_blkCount = blkCount;
    m_blockInfos.reserve(blkCount);
    for (int i=0 ; i<blkCount ; ++i)
    {
        m_blockInfos.emplace_back();
        BlockInfo& info = m_blockInfos.back();
        info.read(source);
        m_totalDecompLen += info.decompSize;
    }
    source.seekAlign32();
    m_secIdxs.reserve(secIdxCount);
    for (int i=0 ; i<secIdxCount ; ++i)
    {
        m_secIdxs.emplace_back();
        std::pair<DNAFourCC, atUint32>& idx = m_secIdxs.back();
        idx.first.read(source);
        idx.second = source.readUint32Big();
    }
    source.seekAlign32();
    m_blkBase = source.position();
    nextBlock();
}

void MREA::StreamReader::writeSecIdxs(Athena::io::IStreamWriter& writer) const
{
    for (const std::pair<DNAFourCC, atUint32>& idx : m_secIdxs)
    {
        idx.first.write(writer);
        writer.writeUint32Big(idx.second);
    }
}

void MREA::ReadBabeDeadToBlender_3(HECL::BlenderConnection::PyOutStream& os,
                                   Athena::io::IStreamReader& rs)
{
    atUint32 bdMagic = rs.readUint32Big();
    if (bdMagic != 0xBABEDEAD)
        Log.report(LogVisor::FatalError, "invalid BABEDEAD magic");
    os << "bpy.context.scene.render.engine = 'CYCLES'\n"
          "bpy.context.scene.world.use_nodes = True\n"
          "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
          "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n";
    for (atUint32 s=0 ; s<4 ; ++s)
    {
        atUint32 lightCount = rs.readUint32Big();
        for (atUint32 l=0 ; l<lightCount ; ++l)
        {
            BabeDeadLight light;
            light.read(rs);
            switch (light.lightType)
            {
            case BabeDeadLight::LightLocalAmbient:
                os.format("bg_node.inputs[0].default_value = (%f,%f,%f,1.0)\n"
                          "bg_node.inputs[1].default_value = %f\n",
                          light.color.vec[0], light.color.vec[1], light.color.vec[2],
                          light.unk6 / 8.0);
                continue;
            case BabeDeadLight::LightDirectional:
                os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'SUN')\n"
                          "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                          "lamp_obj.rotation_mode = 'QUATERNION'\n"
                          "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
                          "\n", s, l,
                          light.direction.vec[0], light.direction.vec[1], light.direction.vec[2]);
                break;
            case BabeDeadLight::LightCustom:
                os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'POINT')\n"
                          "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                          "\n", s, l);
                break;
            case BabeDeadLight::LightSpot:
                os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'SPOT')\n"
                          "lamp.spot_size = 1.0\n"
                          "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                          "lamp_obj.rotation_mode = 'QUATERNION'\n"
                          "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
                          "\n", s, l,
                          light.direction.vec[0], light.direction.vec[1], light.direction.vec[2]);
                break;
            default: continue;
            }

            os.format("lamp.retro_layer = %u\n"
                      "lamp.use_nodes = True\n"
                      "quadratic_node = lamp.node_tree.nodes.new('ShaderNodeLightFalloff')\n"
                      "quadratic_node.inputs[0].default_value = %f\n"
                      "quadratic_node.location = (-600, 0)\n"
                      "linear_node = lamp.node_tree.nodes.new('ShaderNodeLightFalloff')\n"
                      "linear_node.inputs[0].default_value = %f\n"
                      "linear_node.location = (-400, 0)\n"
                      "constant_node = lamp.node_tree.nodes.new('ShaderNodeLightFalloff')\n"
                      "constant_node.inputs[0].default_value = %f\n"
                      "constant_node.location = (-200, 0)\n"
                      "add1 = lamp.node_tree.nodes.new('ShaderNodeMath')\n"
                      "add1.operation = 'ADD'\n"
                      "add1.location = (-400, -300)\n"
                      "add2 = lamp.node_tree.nodes.new('ShaderNodeMath')\n"
                      "add2.operation = 'ADD'\n"
                      "add2.location = (-200, -300)\n"
                      "lamp.node_tree.links.new(quadratic_node.outputs[0], add1.inputs[0])\n"
                      "lamp.node_tree.links.new(linear_node.outputs[1], add1.inputs[1])\n"
                      "lamp.node_tree.links.new(add1.outputs[0], add2.inputs[0])\n"
                      "lamp.node_tree.links.new(constant_node.outputs[2], add2.inputs[1])\n"
                      "lamp.node_tree.links.new(add2.outputs[0], lamp.node_tree.nodes['Emission'].inputs[1])\n"
                      "lamp.energy = 0.0\n"
                      "hue_sat_node = lamp.node_tree.nodes.new('ShaderNodeHueSaturation')\n"
                      "hue_sat_node.inputs[1].default_value = 1.25\n"
                      "hue_sat_node.inputs[4].default_value = (%f,%f,%f,1.0)\n"
                      "lamp.node_tree.links.new(hue_sat_node.outputs[0], lamp.node_tree.nodes['Emission'].inputs[0])\n"
                      "lamp_obj.location = (%f,%f,%f)\n"
                      "bpy.context.scene.objects.link(lamp_obj)\n"
                      "\n", s, light.unk5, light.unk6, light.unk7,
                      light.color.vec[0], light.color.vec[1], light.color.vec[2],
                      light.position.vec[0], light.position.vec[1], light.position.vec[2]);
        }
    }
}

bool MREA::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const HECL::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool,
                   std::function<void(const HECL::SystemChar*)>)
{
    using RigPair = std::pair<CSKR*, CINF*>;
    RigPair dummy(nullptr, nullptr);

    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    /* MREA decompression stream */
    StreamReader drs(rs, head.compressedBlockCount, head.secIndexCount);
    Athena::io::FileWriter mreaDecompOut(pakRouter.getCooked(&entry).getWithExtension(_S(".decomp")).getAbsolutePath());
    head.write(mreaDecompOut);
    mreaDecompOut.seekAlign32();
    drs.writeDecompInfos(mreaDecompOut);
    mreaDecompOut.seekAlign32();
    drs.writeSecIdxs(mreaDecompOut);
    mreaDecompOut.seekAlign32();
    atUint64 decompLen = drs.length();
    mreaDecompOut.writeBytes(drs.readBytes(decompLen).get(), decompLen);
    mreaDecompOut.close();
    drs.seek(0, Athena::Begin);


    /* Start up blender connection */
    HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
    if (!conn.createBlend(outPath.getAbsolutePath()))
        return false;

    /* Open Py Stream and read sections */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Vector\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.hecl_type = 'AREA'\n",
              pakRouter.getBestEntryName(entry).c_str());
    DNACMDL::InitGeomBlenderContext(os, dataSpec.getMasterShaderPath());
    MaterialSet::RegisterMaterialProps(os);
    os << "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    bpy.context.scene.objects.unlink(ob)\n"
          "    bpy.data.objects.remove(ob)\n"
          "bpy.types.Lamp.retro_layer = bpy.props.IntProperty(name='Retro: Light Layer')\n"
          "\n";

    /* One shared material set for all meshes */
    os << "# Materials\n"
          "materials = []\n"
          "\n";
    MaterialSet matSet;
    atUint64 secStart = drs.position();
    matSet.read(drs);
    matSet.readToBlender(os, pakRouter, entry, 0);
    drs.seek(secStart + head.secSizes[0], Athena::Begin);
    std::vector<DNACMDL::VertexAttributes> vertAttribs;
    DNACMDL::GetVertexAttributes(matSet, vertAttribs);

    /* Read mesh info */
    atUint32 curSec = 1;
    std::vector<atUint32> surfaceCounts;
    surfaceCounts.reserve(head.meshCount);
    for (int m=0 ; m<head.meshCount ; ++m)
    {
        /* Mesh header */
        MeshHeader mHeader;
        secStart = drs.position();
        mHeader.read(drs);
        drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

        /* Surface count from here */
        secStart = drs.position();
        surfaceCounts.push_back(drs.readUint32Big());
        drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

        /* Seek through AROT-relation sections */
        drs.seek(head.secSizes[curSec++], Athena::Current);
        drs.seek(head.secSizes[curSec++], Athena::Current);
    }

    /* Skip though WOBJs */
    auto secIdxIt = drs.beginSecIdxs();
    while (secIdxIt->first == FOURCC('WOBJ'))
        ++secIdxIt;

    /* Skip AROT */
    if (secIdxIt->first == FOURCC('ROCT'))
    {
        drs.seek(head.secSizes[curSec++], Athena::Current);
        ++secIdxIt;
    }

    /* Skip AABB */
    if (secIdxIt->first == FOURCC('AABB'))
    {
        drs.seek(head.secSizes[curSec++], Athena::Current);
        ++secIdxIt;
    }

    /* Now the meshes themselves */
    if (secIdxIt->first == FOURCC('GPUD'))
    {
        for (int m=0 ; m<head.meshCount ; ++m)
        {
            curSec += DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_3>
                          (os, drs, pakRouter, entry, dummy, true,
                           false, vertAttribs, m, head.secCount, 0, &head.secSizes[curSec], surfaceCounts[m]);
        }
        ++secIdxIt;
    }

    /* Skip DEPS */
    if (secIdxIt->first == FOURCC('DEPS'))
    {
        drs.seek(head.secSizes[curSec++], Athena::Current);
        ++secIdxIt;
    }

    /* Skip SOBJ (SCLY) */
    if (secIdxIt->first == FOURCC('SOBJ'))
    {
        for (int l=0 ; l<head.sclyLayerCount ; ++l)
            drs.seek(head.secSizes[curSec++], Athena::Current);
        ++secIdxIt;
    }

    /* Skip SGEN */
    if (secIdxIt->first == FOURCC('SGEN'))
    {
        drs.seek(head.secSizes[curSec++], Athena::Current);
        ++secIdxIt;
    }

    /* Read Collision Meshes */
    if (secIdxIt->first == FOURCC('COLI'))
    {
        DNAMP2::DeafBabe collision;
        secStart = drs.position();
        collision.read(drs);
        DNAMP2::DeafBabe::BlenderInit(os);
        collision.sendToBlender(os);
        drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);
        ++secIdxIt;
    }

    /* Read BABEDEAD Lights as Cycles emissives */
    if (secIdxIt->first == FOURCC('LITE'))
    {
        secStart = drs.position();
        ReadBabeDeadToBlender_3(os, drs);
        drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);
        ++secIdxIt;
    }

    /* Origins to center of mass */
    os << "bpy.context.scene.layers[1] = True\n"
          "bpy.ops.object.select_by_type(type='MESH')\n"
          "bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')\n"
          "bpy.ops.object.select_all(action='DESELECT')\n"
          "bpy.context.scene.layers[1] = False\n";

    /* Center view */
    os << "bpy.context.user_preferences.view.smooth_view = 0\n"
          "for window in bpy.context.window_manager.windows:\n"
          "    screen = window.screen\n"
          "    for area in screen.areas:\n"
          "        if area.type == 'VIEW_3D':\n"
          "            for region in area.regions:\n"
          "                if region.type == 'WINDOW':\n"
          "                    override = {'scene': bpy.context.scene, 'window': window, 'screen': screen, 'area': area, 'region': region}\n"
          "                    bpy.ops.view3d.view_all(override)\n"
          "                    break\n";

    os.close();
    return conn.saveBlend();
}

bool MREA::ExtractLayerDeps(PAKEntryReadStream& rs, PAKBridge::Level::Area& areaOut)
{
    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    /* MREA decompression stream */
    StreamReader drs(rs, head.compressedBlockCount, head.secIndexCount);
    for (const std::pair<DNAFourCC, atUint32>& idx : drs.m_secIdxs)
    {
        if (idx.first == FOURCC('DEPS'))
        {
            drs.seek(head.getSecOffset(idx.second), Athena::Begin);
            DEPS deps;
            deps.read(drs);

            unsigned r=0;
            for (unsigned l=1 ; l<deps.depLayerCount ; ++l)
            {
                if (l > areaOut.layers.size())
                    break;
                PAKBridge::Level::Area::Layer& layer = areaOut.layers.at(l-1);
                layer.resources.reserve(deps.depLayers[l] - r);
                for (; r<deps.depLayers[l] ; ++r)
                    layer.resources.emplace(deps.deps[r].id);
            }
            areaOut.resources.reserve(deps.depCount - r);
            for (; r<deps.depCount ; ++r)
                areaOut.resources.emplace(deps.deps[r].id);

            return true;
        }
    }
    return false;
}

}
}
