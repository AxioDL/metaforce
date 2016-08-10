#include "MREA.hpp"
#include "SCLY.hpp"
#include "DeafBabe.hpp"
#include "../DNACommon/BabeDead.hpp"
#include "zeus/Math.hpp"
#include "zeus/CAABox.hpp"
#include "DataSpec/DNACommon/AROTBuilder.hpp"

namespace DataSpec
{
namespace DNAMP1
{

void MREA::ReadBabeDeadToBlender_1_2(hecl::BlenderConnection::PyOutStream& os,
                                     athena::io::IStreamReader& rs)
{
    atUint32 bdMagic = rs.readUint32Big();
    if (bdMagic != 0xBABEDEAD)
        Log.report(logvisor::Fatal, "invalid BABEDEAD magic");
    os << "bpy.context.scene.render.engine = 'CYCLES'\n"
          "bpy.context.scene.world.use_nodes = True\n"
          "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
          "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n";
    for (atUint32 s=0 ; s<2 ; ++s)
    {
        atUint32 lightCount = rs.readUint32Big();
        for (atUint32 l=0 ; l<lightCount ; ++l)
        {
            BabeDeadLight light;
            light.read(rs);
            ReadBabeDeadLightToBlender(os, light, s, l);
        }
    }
}

void MREA::AddCMDLRigPairs(PAKEntryReadStream& rs,
                           PAKRouter<PAKBridge>& pakRouter,
                           std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo)
{
    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    /* Skip to SCLY */
    atUint32 curSec = 0;
    atUint64 secStart = rs.position();
    while (curSec != head.sclySecIdx)
        secStart += head.secSizes[curSec++];
    rs.seek(secStart, athena::Begin);
    SCLY scly;
    scly.read(rs);
    scly.addCMDLRigPairs(pakRouter, addTo);
}

bool MREA::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::BlenderToken& btok,
                   std::function<void(const hecl::SystemChar*)>)
{
    using RigPair = std::pair<CSKR*, CINF*>;
    RigPair dummy(nullptr, nullptr);

    hecl::ProjectPath mreaPath;
    if (pakRouter.isShared())
        /* Rename MREA for consistency */
        mreaPath = hecl::ProjectPath(outPath.getParentPath(), _S("!area.blend"));
    else
        /* We're not in a world pak, so lets keep the original name */
        mreaPath = outPath;

    if (!force && mreaPath.getPathType() == hecl::ProjectPath::Type::File)
        return true;

    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    hecl::BlenderConnection& conn = btok.getBlenderConnection();
    if (!conn.createBlend(mreaPath, hecl::BlenderConnection::BlendType::Area))
        return false;

    /* Open Py Stream and read sections */
    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Vector\n"
              "\n"
              "bpy.context.scene.name = '%s'\n",
              pakRouter.getBestEntryName(entry).c_str());
    DNACMDL::InitGeomBlenderContext(os, dataSpec.getMasterShaderPath());
    MaterialSet::RegisterMaterialProps(os);
    os << "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    if ob.type != 'CAMERA':\n"
          "        bpy.context.scene.objects.unlink(ob)\n"
          "        bpy.data.objects.remove(ob)\n"
          "bpy.types.Lamp.retro_layer = bpy.props.IntProperty(name='Retro: Light Layer')\n"
          "bpy.types.Lamp.retro_origtype = bpy.props.IntProperty(name='Retro: Original Type')\n"
          "bpy.types.Object.retro_disable_enviro_visor = bpy.props.BoolProperty(name='Retro: Disable in Combat/Scan Visor')\n"
          "bpy.types.Object.retro_disable_thermal_visor = bpy.props.BoolProperty(name='Retro: Disable in Thermal Visor')\n"
          "bpy.types.Object.retro_disable_xray_visor = bpy.props.BoolProperty(name='Retro: Disable in X-Ray Visor')\n"
          "bpy.types.Object.retro_thermal_level = bpy.props.EnumProperty(items=[('COOL', 'Cool', 'Cool Temperature'),"
                                                                               "('HOT', 'Hot', 'Hot Temperature'),"
                                                                               "('WARM', 'Warm', 'Warm Temperature')],"
                                                                        "name='Retro: Thermal Visor Level')\n"
          "\n";

    /* One shared material set for all meshes */
    os << "# Materials\n"
          "materials = []\n"
          "\n";
    MaterialSet matSet;
    atUint64 secStart = rs.position();
    matSet.read(rs);
    matSet.readToBlender(os, pakRouter, entry, 0);
    rs.seek(secStart + head.secSizes[0], athena::Begin);
    std::vector<DNACMDL::VertexAttributes> vertAttribs;
    DNACMDL::GetVertexAttributes(matSet, vertAttribs);

    /* Read meshes */
    atUint32 curSec = 1;
    for (atUint32 m=0 ; m<head.meshCount ; ++m)
    {
        MeshHeader mHeader;
        secStart = rs.position();
        mHeader.read(rs);
        rs.seek(secStart + head.secSizes[curSec++], athena::Begin);
        curSec += DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_1>
                      (os, rs, pakRouter, entry, dummy, true,
                       true, vertAttribs, m, head.secCount, 0, &head.secSizes[curSec]);
        os.format("obj.retro_disable_enviro_visor = %s\n"
                  "obj.retro_disable_thermal_visor = %s\n"
                  "obj.retro_disable_xray_visor = %s\n"
                  "obj.retro_thermal_level = '%s'\n",
                  mHeader.visorFlags.disableEnviro() ? "True" : "False",
                  mHeader.visorFlags.disableThermal() ? "True" : "False",
                  mHeader.visorFlags.disableXray() ? "True" : "False",
                  mHeader.visorFlags.thermalLevelStr());
    }

    /* Skip AROT */
    rs.seek(head.secSizes[curSec++], athena::Current);

    /* Read SCLY layers */
    secStart = rs.position();
    SCLY scly;
    scly.read(rs);
    scly.exportToLayerDirectories(entry, pakRouter, force);
    rs.seek(secStart + head.secSizes[curSec++], athena::Begin);

    /* Read collision meshes */
    DeafBabe collision;
    secStart = rs.position();
    collision.read(rs);
    DeafBabe::BlenderInit(os);
    collision.sendToBlender(os);
    rs.seek(secStart + head.secSizes[curSec++], athena::Begin);

    /* Skip unknown section */
    rs.seek(head.secSizes[curSec++], athena::Current);

    /* Read BABEDEAD Lights as Cycles emissives */
    secStart = rs.position();
    ReadBabeDeadToBlender_1_2(os, rs);
    rs.seek(secStart + head.secSizes[curSec++], athena::Begin);

    /* Origins to center of mass */
    os << "bpy.context.scene.layers[1] = True\n"
          "bpy.ops.object.select_by_type(type='MESH')\n"
          "bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')\n"
          "bpy.ops.object.select_all(action='DESELECT')\n"
          "bpy.context.scene.layers[1] = False\n";

    /* Link MLVL scene as background */
    os.linkBackground("//../!world.blend", "World");

    os.centerView();
    os.close();
    return conn.saveBlend();
}

void MREA::Name(const SpecBase& dataSpec,
                PAKEntryReadStream& rs,
                PAKRouter<PAKBridge>& pakRouter,
                PAK::Entry& entry)
{
    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    /* One shared material set for all meshes */
    atUint64 secStart = rs.position();
    MaterialSet matSet;
    matSet.read(rs);
    matSet.nameTextures(pakRouter, hecl::Format("MREA_%s", entry.id.toString().c_str()).c_str(), -1);
    rs.seek(secStart + head.secSizes[0], athena::Begin);

    /* Skip to SCLY */
    atUint32 curSec = 1;
    secStart = rs.position();
    while (curSec != head.sclySecIdx)
        secStart += head.secSizes[curSec++];
    rs.seek(secStart, athena::Begin);
    SCLY scly;
    scly.read(rs);
    scly.nameIDs(pakRouter);

    /* Skip to PATH */
    while (curSec != head.pathSecIdx)
        secStart += head.secSizes[curSec++];
    rs.seek(secStart, athena::Begin);

    UniqueID32 pathID(rs);
    const nod::Node* node;
    PAK::Entry* pathEnt = (PAK::Entry*)pakRouter.lookupEntry(pathID, &node);
    pathEnt->name = entry.name + "_path";
}

void MREA::MeshHeader::VisorFlags::setFromBlenderProps(const std::unordered_map<std::string, std::string>& props)
{
    auto search = props.find("retro_disable_enviro_visor");
    if (search != props.cend() && !search->second.compare("True"))
        setDisableEnviro(true);
    search = props.find("retro_disable_thermal_visor");
    if (search != props.cend() && !search->second.compare("True"))
        setDisableThermal(true);
    search = props.find("retro_disable_xray_visor");
    if (search != props.cend() && !search->second.compare("True"))
        setDisableXray(true);
    search = props.find("retro_thermal_level");
    if (search != props.cend())
    {
        if (!search->second.compare("COOL"))
            setThermalLevel(ThermalLevel::Cool);
        else if (!search->second.compare("HOT"))
            setThermalLevel(ThermalLevel::Hot);
        else if (!search->second.compare("WARM"))
            setThermalLevel(ThermalLevel::Warm);
    }
}

bool MREA::Cook(const hecl::ProjectPath& outPath,
                const hecl::ProjectPath& inPath,
                const std::vector<DNACMDL::Mesh>& meshes,
                const ColMesh& cMesh)
{
    return false;
}

bool MREA::PCCook(const hecl::ProjectPath& outPath,
                  const hecl::ProjectPath& inPath,
                  const std::vector<DNACMDL::Mesh>& meshes,
                  const ColMesh& cMesh)
{
    /* Discover area layers */
    hecl::ProjectPath areaDirPath = inPath.getParentPath();
    std::vector<hecl::ProjectPath> layerScriptPaths;
    {
        hecl::DirectoryEnumerator dEnum(inPath.getParentPath().getAbsolutePath(),
                                        hecl::DirectoryEnumerator::Mode::DirsSorted,
                                        false, false, true);
        for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
        {
            hecl::ProjectPath layerScriptPath(areaDirPath, ent.m_name + _S("/objects.yaml"));
            if (layerScriptPath.getPathType() == hecl::ProjectPath::Type::File)
                layerScriptPaths.push_back(std::move(layerScriptPath));
        }
    }

    size_t secCount = 1 + meshes.size() * 5; /* (materials, 5 fixed model secs) */

    /* tally up surfaces */
    for (const DNACMDL::Mesh& mesh : meshes)
        secCount += mesh.surfaces.size();

    /* Header */
    Header head = {};
    head.magic = 0xDEADBEEF;
    head.version = 0x1000F;
    head.localToWorldMtx[0].vec[0] = 1.f;
    head.localToWorldMtx[1].vec[1] = 1.f;
    head.localToWorldMtx[2].vec[2] = 1.f;
    head.meshCount = meshes.size();
    head.geomSecIdx = 0;
    head.arotSecIdx = secCount++;
    head.sclySecIdx = secCount++;
    head.collisionSecIdx = secCount++;
    head.unkSecIdx = secCount++;
    head.lightSecIdx = secCount++;
    head.visiSecIdx = secCount++;
    head.pathSecIdx = secCount++;
    head.secCount = secCount;

    std::vector<std::vector<uint8_t>> secs;
    secs.reserve(secCount + 2);

    /* Header section */
    {
        secs.emplace_back(head.binarySize(0), 0);
        athena::io::MemoryWriter w(secs.back().data(), secs.back().size());
        head.write(w);
        int i = w.position();
        int end = ROUND_UP_32(i);
        for (; i<end ; ++i)
            w.writeByte(0);
    }

    /* Sizes section */
    secs.emplace_back();
    std::vector<uint8_t>& sizesSec = secs.back();

    /* Pre-emptively build full AABB and mesh AABBs in world coords */
    zeus::CAABox fullAabb;
    std::vector<zeus::CAABox> meshAabbs;
    meshAabbs.reserve(meshes.size());

    /* Models */
    if (!DNACMDL::WriteHMDLMREASecs<HMDLMaterialSet, DNACMDL::SurfaceHeader_2, MeshHeader>
            (secs, inPath, meshes, fullAabb, meshAabbs))
        return false;

    /* AROT */
    {
        AROTBuilder builder;
        builder.build(secs, fullAabb, meshAabbs, meshes);
    }

    /* SCLY */
    {
        DNAMP1::SCLY sclyData;
        sclyData.fourCC = FOURCC('SCLY');
        sclyData.version = 1;
        for (const hecl::ProjectPath& layer : layerScriptPaths)
        {
            FILE* yamlFile = hecl::Fopen(layer.getAbsolutePath().c_str(), _S("r"));
            if (!yamlFile)
                continue;
            if (!BigYAML::ValidateFromYAMLFile<DNAMP1::SCLY>(yamlFile))
            {
                fclose(yamlFile);
                continue;
            }

            athena::io::YAMLDocReader reader;
            yaml_parser_set_input_file(reader.getParser(), yamlFile);
            if (!reader.parse())
            {
                fclose(yamlFile);
                continue;
            }
            fclose(yamlFile);

            sclyData.layers.emplace_back();
            sclyData.layers.back().read(reader);
            sclyData.layerSizes.push_back(sclyData.layers.back().binarySize(0));
        }
        sclyData.layerCount = sclyData.layers.size();

        secs.emplace_back(sclyData.binarySize(0), 0);
        athena::io::MemoryWriter w(secs.back().data(), secs.back().size());
        sclyData.write(w);
    }


    /* Collision */


    /* Unk */

    /* Lights */

    /* VISI */

    /* PATH */

    /* Assemble sizes and add padding */
    {
        sizesSec.assign(head.secCount, 0);
        int totalEnd = sizesSec.size() * 4;
        int totalPadEnd = ROUND_UP_32(totalEnd);
        athena::io::MemoryWriter w(sizesSec.data(), totalPadEnd);
        for (auto it = secs.begin() + 2 ; it != secs.end() ; ++it)
        {
            std::vector<uint8_t>& sec = *it;
            int i = sec.size();
            int end = ROUND_UP_32(i);
            for (; i<end ; ++i)
                sec.push_back(0);
            w.writeUint32Big(end);
        }
    }

    /* Output all padded sections to file */
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    for (const std::vector<uint8_t>& sec : secs)
        writer.writeUBytes(sec.data(), sec.size());

    return true;
}

}
}
